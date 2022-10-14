#include "debug.h"
#include "dump.h"
#include "matte/src/matte_vm.h"
#include "matte/src/matte_string.h"
#include "matte/src/matte_array.h"
#include "matte/src/matte_table.h"
#include "matte/src/matte_bytecode_stub.h"
#include "native.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>


typedef struct {
    int enabled;
    int active;
    int requestedExit;
    
    int currentLine;
    uint32_t currentFileid;
    char * directory;
    
    matteValue_t onPrint;
    matteValue_t onClear;
    matteValue_t activateConsole;
    matteHeap_t * heap;
    matteVM_t * vm;
    matte_t * matte;
    
    matteTable_t * files;
    
    
    uint32_t callstackLevel;
    uint32_t callstackLimit;
    
} SESDebug;


static SESDebug debug = {};

typedef enum {
    SESDebug_Color__Normal,
    SESDebug_Color__Code,
    SESDebug_Color__Error,
} SESDebug_Color;

// prints to the debug console once active.
static void debug_println(const char * format, int colorHint, ...) {
    
    const int MAX_STR_LEN = 2048;
    char * text = malloc(MAX_STR_LEN+1);
    text[0] = 0;
    va_list args = {};
    va_start(args, colorHint);
    vsnprintf(text, MAX_STR_LEN, format, args);
    va_end(args);
    
    matteString_t * str = matte_string_create_from_c_str(text);
    printf("%s\n", text);
    free(text);text = NULL;
    matteValue_t strval = matte_heap_new_value(debug.heap);
    matte_value_into_string(debug.heap, &strval, str);
    matte_string_destroy(str);


    matteString_t * textStr = (matteString_t*)MATTE_VM_STR_CAST(debug.vm, "text");
    matteValue_t textval = matte_heap_new_value(debug.heap);
    matte_value_into_string(debug.heap, &textval, textStr);


    matteValue_t colhnum = matte_heap_new_value(debug.heap);
    matte_value_into_number(debug.heap, &colhnum, colorHint);


    matteString_t * colhStr = (matteString_t*)MATTE_VM_STR_CAST(debug.vm, "colorHint");
    matteValue_t colhval = matte_heap_new_value(debug.heap);
    matte_value_into_string(debug.heap, &colhval, colhStr);



    matteValue_t namesArr[] = {textval, colhval};
    matteValue_t valsArr[] = {strval, colhnum};                

    matteArray_t names = MATTE_ARRAY_CAST(namesArr, matteValue_t, 2);
    matteArray_t vals = MATTE_ARRAY_CAST(valsArr, matteValue_t, 2);
    
    matte_vm_call(debug.vm, debug.onPrint, &vals, &names, NULL);
    
}


static void debug_clear() {
    matte_vm_call(debug.vm, debug.onClear, matte_array_empty(), matte_array_empty(), NULL);
    
    
}


static void ses_matte_debug_event(
    matteVM_t * vm, 
    matteVMDebugEvent_t event, 
    uint32_t file, 
    int lineNumber, 
    matteValue_t value, 
    void * data
) {
    debug.currentFileid = file;
    debug.currentLine = lineNumber;
}


static void ses_matte_query__error(
    matteVM_t * vm, 
    matteVMDebugEvent_t event, 
    uint32_t file, 
    int lineNumber, 
    matteValue_t value, 
    void * data
) {
    if (event == MATTE_VM_DEBUG_EVENT__ERROR_RAISED) {
        if (value.binID == MATTE_VALUE_TYPE_STRING) {
            debug_println("Error: %s", SESDebug_Color__Error, matte_string_get_c_str(matte_value_string_get_string_unsafe(debug.heap, value)));
        }
    }
}


static matteArray_t * split_lines(const uint8_t * data, uint32_t size) {
    matteArray_t * localLines = matte_array_create(sizeof(matteString_t *));
    uint32_t i;
    matteString_t * line = matte_string_create();
    for(i = 0; i < size; ++i) {
        if (data[i] == '\n') {
            matte_array_push(localLines, line);
            line = matte_string_create();
        } else {
            if (data[i] != '\r')
                matte_string_append_char(line, data[i]);
        }
    }
    return localLines;
}



static void ses_matte_backtrace() {
    uint32_t i;
    uint32_t len = matte_vm_get_stackframe_size(debug.vm);
    
    if (len < 1) {
        debug_println("<stackframe empty>", SESDebug_Color__Code);
        return;
    }
    
    uint32_t start = len - debug.callstackLimit;
    for(i = start; i < len; ++i) {
        matteVMStackFrame_t frame = matte_vm_get_stackframe(debug.vm, i);
        uint32_t fileid = matte_bytecode_stub_get_file_id(frame.stub);
        uint32_t instCount;
        const matteBytecodeStubInstruction_t * inst = matte_bytecode_stub_get_instructions(frame.stub, &instCount);
        uint32_t lineNumber = inst[frame.pc].lineNumber;
                
        
        
        const matteString_t * filename = matte_vm_get_script_name_by_id(debug.vm, fileid);
        if (filename == NULL) {
            debug_println("%s????: %d", SESDebug_Color__Code, (i-start == debug.callstackLevel ? "> " : "  "),  lineNumber);        
        } else {
            debug_println("%s%s: %d", SESDebug_Color__Code, (i-start == debug.callstackLevel ? "> " : "  "), matte_string_get_c_str(filename), lineNumber);
        }    
        
    }
}

static int ses_matte_debug_dump() {
    matteVM_t * vm = debug.vm;        
    matteVMStackFrame_t frame = matte_vm_get_stackframe(vm, debug.callstackLevel + (matte_vm_get_stackframe_size(vm) - debug.callstackLimit));
    uint32_t fileid = matte_bytecode_stub_get_file_id(frame.stub);

    const matteString_t * name = matte_vm_get_script_name_by_id(debug.vm, fileid);
    if (!name) goto L_FAIL;
    matteString_t * path = matte_string_create_from_c_str("%s/%s", debug.directory, matte_string_get_c_str(name));
    matteArray_t  * data = matte_table_find(debug.files, path);
    if (!data) {
        uint32_t rawlen;
        uint8_t * raw = dump_bytes(matte_string_get_c_str(path), &rawlen);
        
        if (raw && rawlen) {
            data = split_lines(raw, rawlen);
            free(raw);
        }

        matte_table_insert(debug.files, path, data);
        
    }

    matte_string_destroy(path);
    
    if (!data)
        goto L_FAIL;

    uint32_t numinst;
    const matteBytecodeStubInstruction_t * inst = matte_bytecode_stub_get_instructions(frame.stub, &numinst);
    uint32_t line = 0;
    if (frame.pc-1 >= 0 && frame.pc-1 < numinst)
        line = inst[frame.pc-1].lineNumber;

    debug_println("<file %s, line %d>", SESDebug_Color__Code, 
        matte_string_get_c_str(name),
        line
    );
    int i = line;

    const int PRINT_AREA_LINES = 7;

    matteArray_t * localLines = data;
    if (localLines) {
        for(i = ((int)line) - PRINT_AREA_LINES/2; i < ((int)line) + PRINT_AREA_LINES/2 + 1; ++i) {
            if (i < 0 || i >= matte_array_get_size(localLines)) {
                debug_println("  ---- | \n", SESDebug_Color__Code);
            } else {
                if (i == line-1) {
                    debug_println("->%4d | %s", 3, i+1, matte_string_get_c_str(matte_array_at(localLines, matteString_t *, i)));
                } else {
                    debug_println("  %4d | %s", SESDebug_Color__Code, i+1, matte_string_get_c_str(matte_array_at(localLines, matteString_t *, i)));
                }
            }
        }
    }     
    return 1;   
    
  L_FAIL:
    debug_println("<File not found>", SESDebug_Color__Error);
    debug_println("", SESDebug_Color__Error);
    ses_matte_backtrace();
  
}



static void ses_debug_unhandled_error(
    matteVM_t * vm, 
    uint32_t file, 
    int lineNumber, 
    matteValue_t val, 
    void * userdata
) {
    
    if (val.binID == MATTE_VALUE_TYPE_OBJECT) {
        matteValue_t s = matte_value_object_access_string(matte_vm_get_heap(vm), val, MATTE_VM_STR_CAST(vm, "summary"));
        if (s.binID) {
            
            debug_println(
                "Unhandled error: %s\n", 
                SESDebug_Color__Error, 
                matte_string_get_c_str(matte_value_string_get_string_unsafe(matte_vm_get_heap(vm), s))
            );
            fflush(stdout);
            return;
        }
    }
    
    debug_println(
        "Unhandled error (%s, line %d)\n",
        SESDebug_Color__Error,  
        matte_string_get_c_str(matte_vm_get_script_name_by_id(vm, file)), 
        lineNumber
    );
}






static matteValue_t ses_native__debug_context_enter(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData){
    debug.active = 1;
    ses_native_swap_context();
    debug.requestedExit = 0;
    
    debug.onPrint = args[0];
    debug.onClear = args[1];
    debug.callstackLevel = 0;
    debug.callstackLimit = matte_vm_get_stackframe_size(debug.vm)-1;
    
    matteHeap_t * heap = matte_vm_get_heap(vm);
    matte_value_object_push_lock(heap, debug.onPrint);    
    matte_value_object_push_lock(heap, debug.onClear);


    matte_vm_call(debug.vm, args[2], matte_array_empty(), matte_array_empty(), NULL);
    
    debug_println("SES. (debug console)", SESDebug_Color__Normal);
    debug_println("http://github.com/jcorks/", SESDebug_Color__Normal);
    debug_println("sprite-entertainment-system", SESDebug_Color__Normal);    
    debug_println("", SESDebug_Color__Normal);
    
    ses_matte_backtrace();
    ses_matte_debug_dump();
}
static matteValue_t ses_native__debug_context_update(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    debug.requestedExit |= !ses_native_update(debug.matte);    
}
static matteValue_t ses_native__debug_context_leave(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    debug.active = 0;
    matte_value_object_pop_lock(debug.heap, debug.onPrint);    
    matte_value_object_pop_lock(debug.heap, debug.onClear);
    
    debug.onPrint.binID = 0;
    debug.onClear.binID = 0;
    ses_native_swap_context();

}
static matteValue_t ses_native__debug_context_query(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    const char * src = matte_string_get_c_str(matte_value_string_get_string_unsafe(debug.heap, args[0]));
    debug_println(src, 3);
    uint32_t len = strlen(src);
    char * query = malloc(len+1);
    memcpy(query, src, len+1);
    while(len && isspace(query[len-1])) {
        query[len-1] = 0;
        len--;
    }


    // continue normal execution
    if (!strcmp(query, ":c") ||
        !strcmp(query, ":continue")) {
        debug.requestedExit = 1;

    // print callstack
    } else if (!strcmp(query, ":bt") ||
        !strcmp(query, ":backtracew")) {
        
        ses_matte_backtrace();

    // up the callstack
    } else if (!strcmp(query, ":up") ||
        !strcmp(query, ":u")) {
        debug.callstackLevel += 1;

        if (debug.callstackLevel >= debug.callstackLimit)
            debug.callstackLevel = debug.callstackLimit-1;
        debug_clear();
        ses_matte_debug_dump();            

    // down the callstack
    } else if (!strcmp(query, ":down") ||
        !strcmp(query, ":d")) {
        if (debug.callstackLevel)
            debug.callstackLevel -= 1;
        debug_clear();

        ses_matte_debug_dump();            
    } else {


        matteString_t * src = matte_string_create();
        matte_string_concat_printf(src, "return import(module:'Matte.Core.Introspect')(value:%s);", query);


        matteValue_t result = matte_vm_run_scoped_debug_source(
            debug.vm,
            src,
            debug.callstackLevel + (matte_vm_get_stackframe_size(debug.vm) - debug.callstackLimit),
            ses_matte_query__error,
            NULL
        );   
        
        if (result.binID == MATTE_VALUE_TYPE_STRING) {
            debug_println("%s", SESDebug_Color__Code, matte_string_get_c_str(matte_value_string_get_string_unsafe(debug.heap, result)));
            
        } else {
            debug_println("  (invalid value)", SESDebug_Color__Error);
        }
    }
    free(query);    
    return matte_heap_new_value(debug.heap);
    
    
}
static matteValue_t ses_native__debug_context_is_done(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    matteValue_t out = matte_heap_new_value(heap);
    
    matte_value_into_boolean(heap, &out, debug.requestedExit);
    return out;
}



static matteValue_t ses_native__debug_context_is_allowed(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    matteValue_t out = matte_heap_new_value(heap);
    
    matte_value_into_boolean(heap, &out, debug.enabled);
    return out;
}




void ses_debug_init(matte_t * m, int enabled, const char * romPath) {
    matteVM_t * vm = matte_get_vm(m);

    debug.enabled = enabled;   
    debug.active = 0;
    debug.heap = matte_vm_get_heap(vm);
    debug.vm = vm;
    debug.matte = m;
    debug.files = matte_table_create_hash_matte_string();
    
    debug.directory = NULL;
    if (romPath) {
        uint32_t len = strlen(romPath);
        uint32_t i;
        for(i = len-1; i > 0; i--) {
            if (romPath[i] == '/' || romPath[i] == '\\') {
                debug.directory = strdup(romPath);
                debug.directory[i+1] = 0; // include the slash.
                break;
            }
        }
        
    }
    
    
    if (debug.enabled) {
        matte_vm_set_unhandled_callback(vm, ses_debug_unhandled_error, NULL);
        matte_vm_set_debug_callback(vm, ses_matte_debug_event, NULL);
    }
    
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__debug_context_is_allowed"), 0, ses_native__debug_context_is_allowed, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__debug_context_is_done"),    0, ses_native__debug_context_is_done, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__debug_context_enter"),      3, ses_native__debug_context_enter, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__debug_context_update"),     0, ses_native__debug_context_update, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__debug_context_leave"),      0, ses_native__debug_context_leave, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__debug_context_query"),      1, ses_native__debug_context_query, NULL);
        
}

