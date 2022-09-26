#include "debug.h"
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
    
} SESDebug;


static SESDebug debug = {};


// prints to the debug console once active.
static void debug_println(const char * format, ...) {
    
    const int MAX_STR_LEN = 2048;
    char * text = malloc(MAX_STR_LEN+1);
    text[0] = 0;
    va_list args = {};
    va_start(args, format);
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

    matteValue_t namesArr[] = {textval};
    matteValue_t valsArr[] = {strval};                

    matteArray_t names = MATTE_ARRAY_CAST(namesArr, matteValue_t, 1);
    matteArray_t vals = MATTE_ARRAY_CAST(valsArr, matteValue_t, 1);
    
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
            debug_println("Error: %s", matte_string_get_c_str(matte_value_string_get_string_unsafe(debug.heap, value)));
        }
    }
}



static int ses_matte_debug_dump() {

    const matteString_t * name = matte_vm_get_script_name_by_id(debug.vm, debug.currentFileid);
    return 0;    
    
}


static void ses_matte_backtrace() {
    uint32_t i;
    uint32_t len = matte_vm_get_stackframe_size(debug.vm);
    
    if (len < 1) {
        debug_println("<stackframe empty>");
        return;
    }
    
    for(i = 0; i < len; ++i) {
        matteVMStackFrame_t frame = matte_vm_get_stackframe(debug.vm, i);
        uint32_t fileid = matte_bytecode_stub_get_file_id(frame.stub);
        uint32_t instCount;
        const matteBytecodeStubInstruction_t * inst = matte_bytecode_stub_get_instructions(frame.stub, &instCount);
        uint32_t lineNumber = inst[frame.pc].lineNumber;
                
        
        
        const matteString_t * filename = matte_vm_get_script_name_by_id(debug.vm, fileid);
        if (filename == NULL) {
            debug_println("%s????: %d", (i == debug.callstackLevel ? "> " : "  "),  lineNumber);        
        } else {
            debug_println("%s%s: %d", (i == debug.callstackLevel ? "> " : "  "), matte_string_get_c_str(filename), lineNumber);
        }    
        
    }
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
                matte_string_get_c_str(matte_value_string_get_string_unsafe(matte_vm_get_heap(vm), s))
            );
            fflush(stdout);
            return;
        }
    }
    
    debug_println(
        "Unhandled error (%s, line %d)\n", 
        matte_string_get_c_str(matte_vm_get_script_name_by_id(vm, file)), 
        lineNumber
    );
}






static matteValue_t ses_native__debug_context_enter(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData){
    debug.active = 1;
    debug.requestedExit = 0;
    
    debug.onPrint = args[0];
    debug.onClear = args[0];
    debug.callstackLevel = 1;
    
    matteHeap_t * heap = matte_vm_get_heap(vm);
    matte_value_object_push_lock(heap, debug.onPrint);    
    matte_value_object_push_lock(heap, debug.onClear);
    
    debug_println("SES. (debug console)");
    debug_println("http://github.com/jcorks/");
    debug_println("sprite-entertainment-system");    
    debug_println("");
    
    if (!ses_matte_debug_dump()) {
        debug_println("<File not found>");
        debug_println("");
        ses_matte_backtrace();
    }
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
}
static matteValue_t ses_native__debug_context_query(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    debug_println(matte_string_get_c_str(matte_value_string_get_string_unsafe(debug.heap, args[0])));



    // continue normal execution
    if (!strcmp(matte_string_get_c_str(matte_value_string_get_string_unsafe(debug.heap, args[0])), "!c\n") ||
        !strcmp(matte_string_get_c_str(matte_value_string_get_string_unsafe(debug.heap, args[0])), "!continue\n")) {
        debug.requestedExit = 1;
        return matte_heap_new_value(debug.heap);
    }

    matteString_t * src = matte_string_create();
    matte_string_concat_printf(src, "return import(module:'Matte.Core.Introspect')(value:%s);", matte_string_get_c_str(matte_value_string_get_string_unsafe(debug.heap, args[0])));


    matteValue_t result = matte_vm_run_scoped_debug_source(
        debug.vm,
        src,
        debug.callstackLevel,
        ses_matte_query__error,
        NULL
    );   
    
    if (result.binID == MATTE_VALUE_TYPE_STRING) {
        debug_println("%s", matte_string_get_c_str(matte_value_string_get_string_unsafe(debug.heap, result)));
        
    } else {
        debug_println("  (invalid value)");
    }
    
    
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
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__debug_context_enter"),      2, ses_native__debug_context_enter, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__debug_context_update"),     0, ses_native__debug_context_update, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__debug_context_leave"),      0, ses_native__debug_context_leave, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__debug_context_query"),      1, ses_native__debug_context_query, NULL);
        
}

