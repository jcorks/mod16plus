#include "debug.h"
#include "dump.h"
#include "matte/src/matte_vm.h"
#include "matte/src/matte_compiler.h"
#include "matte/src/matte_bytecode_stub.h"
#include "matte/src/matte_string.h"
#include "matte/src/matte_array.h"
#include "matte/src/matte_table.h"
#include "matte/src/matte_heap.h"
#include "matte/src/matte_bytecode_stub.h"
#include "native.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    int enabled;
    int active;
    int requestedExit;
    
    int currentLine;
    uint32_t currentFileid;
    char * directory;
    matteString_t * promptConsole;
    
    matteValue_t onPrint;
    matteValue_t onClear;
    matteValue_t onCommit;
    matteValue_t onEnter;
    matteValue_t onLeave;
    matteValue_t activateConsole;
    matteHeap_t * heap;
    matteVM_t * vm;
    matte_t * matte;
    
    matteTable_t * files;
    
    
    uint32_t callstackLevel;
    uint32_t callstackLimit;
    // flag for when the debug context is not started by a Debug.breakpoint();
    int noFakeRoot;
    
} MOD16Debug;


static MOD16Debug debug = {};

typedef enum {
    MOD16Debug_Color__Normal,
    MOD16Debug_Color__Code,
    MOD16Debug_Color__Error,
} MOD16Debug_Color;

// prints to the debug console once active.
static void debug_println(const char * format, int colorHint, ...) {
    
    const int MAX_STR_LEN = 2048;
    char * text = malloc(MAX_STR_LEN+1);
    text[0] = 0;
    va_list args;
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

static void debug_show_text() {
    matte_vm_call(debug.vm, debug.onCommit, matte_array_empty(), matte_array_empty(), NULL);
}


static void debug_clear() {
    matte_vm_call(debug.vm, debug.onClear, matte_array_empty(), matte_array_empty(), NULL);
    
    
}


static matteValue_t mod16_native__debug_context_enter(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
static void mod16_matte_debug_event(
    matteVM_t * vm, 
    matteVMDebugEvent_t event, 
    uint32_t file, 
    int lineNumber, 
    matteValue_t value, 
    void * data
) {
    debug.currentFileid = file;
    debug.currentLine = lineNumber;

    if (event == MATTE_VM_DEBUG_EVENT__ERROR_RAISED) {
        if (!debug.active) {
        
            matte_string_set(debug.promptConsole, matte_value_string_get_string_unsafe(matte_vm_get_heap(vm), matte_value_as_string(matte_vm_get_heap(vm), value)));       
            debug.noFakeRoot = 1;
            mod16_native__debug_context_enter(debug.vm, matte_heap_new_value(debug.heap), NULL, NULL);
        }

    }

}


static void mod16_matte_query__error(
    matteVM_t * vm, 
    matteVMDebugEvent_t event, 
    uint32_t file, 
    int lineNumber, 
    matteValue_t value, 
    void * data
) {
    if (event == MATTE_VM_DEBUG_EVENT__ERROR_RAISED) {
        if (value.binID == MATTE_VALUE_TYPE_STRING) {
            debug_println("Error: %s", MOD16Debug_Color__Error, matte_string_get_c_str(matte_value_string_get_string_unsafe(debug.heap, value)));
        }
        debug_show_text();


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



static void mod16_matte_backtrace() {
    uint32_t i;
    uint32_t len = matte_vm_get_stackframe_size(debug.vm);
   
   


    debug_println("(paused) Backtrace: ", MOD16Debug_Color__Normal);
   
    
    if (len < 1) {
        debug_println("<stackframe empty>", MOD16Debug_Color__Code);
        return;
    }
    
    uint32_t start = len - debug.callstackLimit;
    for(i = start; i < len; ++i) {
        matteVMStackFrame_t frame = matte_vm_get_stackframe(debug.vm, i);
        uint32_t fileid = matte_bytecode_stub_get_file_id(frame.stub);
        uint32_t instCount;
        const matteBytecodeStubInstruction_t * inst = matte_bytecode_stub_get_instructions(frame.stub, &instCount);
        
        uint32_t lineNumber;
        if (frame.pc < 0 || frame.pc >= instCount) 
            lineNumber = 0;
        else
            lineNumber = inst[frame.pc].lineNumber;
                
        
        
        const matteString_t * filename = matte_vm_get_script_name_by_id(debug.vm, fileid);
        if (filename == NULL) {
            debug_println("%s????: %d", MOD16Debug_Color__Code, (i-start == debug.callstackLevel ? " ->" : "   "),  lineNumber);        
        } else {
            debug_println("%s%s: %d", MOD16Debug_Color__Code, (i-start == debug.callstackLevel ? " ->" : "   "), matte_string_get_c_str(filename), lineNumber);
        }    
        
    }
    debug_show_text();

}

static int mod16_matte_debug_dump() {
    mod16_matte_backtrace();

    return 1;
    matteVM_t * vm = debug.vm;        
    matteVMStackFrame_t frame = matte_vm_get_stackframe(vm, debug.callstackLevel + (matte_vm_get_stackframe_size(vm) - debug.callstackLimit));
    if (!frame.stub)
        goto L_FAIL;
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

    debug_println("<file %s, line %d>", MOD16Debug_Color__Code, 
        matte_string_get_c_str(name),
        line
    );
    int i = line;

    const int PRINT_AREA_LINES = 7;

    matteArray_t * localLines = data;
    if (localLines) {
        for(i = ((int)line) - PRINT_AREA_LINES/2; i < ((int)line) + PRINT_AREA_LINES/2 + 1; ++i) {
            if (i < 0 || i >= matte_array_get_size(localLines)) {
                debug_println("  ---- | \n", MOD16Debug_Color__Code);
            } else {
                if (i == line-1) {
                    debug_println("->%4d | %s", 3, i+1, matte_string_get_c_str(matte_array_at(localLines, matteString_t *, i)));
                } else {
                    debug_println("  %4d | %s", MOD16Debug_Color__Code, i+1, matte_string_get_c_str(matte_array_at(localLines, matteString_t *, i)));
                }
            }
        }
    }  
    debug_show_text();   
    return 1;   
    
  L_FAIL:
    debug_println("<File not found>", MOD16Debug_Color__Error);
    debug_println("", MOD16Debug_Color__Error);
    mod16_matte_backtrace();
    debug_show_text();
  
}








static matteValue_t mod16_native__debug_context_bind(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData){
    debug.onPrint  = args[0];
    debug.onClear  = args[1];
    debug.onEnter  = args[2];
    debug.onCommit = args[3];
    debug.onLeave  = args[4];


    matte_value_object_push_lock(debug.heap, debug.onPrint);    
    matte_value_object_push_lock(debug.heap, debug.onClear);
    matte_value_object_push_lock(debug.heap, debug.onCommit);
    matte_value_object_push_lock(debug.heap, debug.onEnter);
    matte_value_object_push_lock(debug.heap, debug.onLeave);


};




static void mod16_native_debug_context_leave() {
    debug.active = 0;
    

    
    matte_vm_call(debug.vm, debug.onLeave, matte_array_empty(), matte_array_empty(), NULL);

    mod16_native_swap_context();

}

static matteValue_t mod16_native__debug_context_enter(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData){
    debug.active = 1;
    mod16_native_swap_context();
    debug.requestedExit = 0;
    
    debug.callstackLevel = 0;
    debug.callstackLimit = matte_vm_get_stackframe_size(debug.vm) + (debug.noFakeRoot ? 0 : -1);
    debug.noFakeRoot = 0;
    
    matteHeap_t * heap = matte_vm_get_heap(vm);


    matte_vm_call(debug.vm, debug.onEnter, matte_array_empty(), matte_array_empty(), NULL);
    
    debug_println("MOD16+ Debugger", MOD16Debug_Color__Normal);
    debug_println("http://github.com/jcorks/mod16plus", MOD16Debug_Color__Normal);
    debug_println("", MOD16Debug_Color__Normal);
    debug_println("[[enter :? for help]]", MOD16Debug_Color__Normal);
    
    
    if (matte_string_get_length(debug.promptConsole)) {
        debug_println("SCRIPT ERROR:", MOD16Debug_Color__Error);
        debug_println(matte_string_get_c_str(debug.promptConsole), MOD16Debug_Color__Error);
        matte_string_clear(debug.promptConsole);
        debug_println("run :bt for a backtrace.", MOD16Debug_Color__Normal);
        debug_show_text();
    } else {
        mod16_matte_backtrace();    
    }



    for(;;) {
        if (debug.requestedExit) break;
        debug.requestedExit |= !mod16_native_update(debug.matte);    

    }
    mod16_native_debug_context_leave();

}

static void mod16_debug_unhandled_error(
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
                MOD16Debug_Color__Error, 
                matte_string_get_c_str(matte_value_string_get_string_unsafe(matte_vm_get_heap(vm), s))
            );
            fflush(stdout);
            debug_show_text();
        }
    } else {
        
        debug_println(
            "Unhandled error (%s, line %d)\n",
            MOD16Debug_Color__Error,  
            matte_string_get_c_str(matte_vm_get_script_name_by_id(vm, file)), 
            lineNumber
        );

        debug_show_text();
    }


}

static matteValue_t mod16_native__debug_context_query(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    const char * src = matte_string_get_c_str(matte_value_string_get_string_unsafe(debug.heap, args[0]));
    debug_println("$%s", 3, src);
    uint32_t len = strlen(src);
    char * query = malloc(len+1);
    char * arg = malloc(len+1);
    memcpy(query, src, len+1);
    
    uint32_t queryLen = 0;
    while(queryLen < len && !isspace(src[queryLen])) {
        query[queryLen++] = src[queryLen];
    }
    query[queryLen] = 0;
    
    arg[0] = 0;
    if (queryLen >= len) {
    } else {
        uint32_t argStart = queryLen;
        while(argStart < len && isspace(src[argStart])) {
            argStart++;
        }
        uint32_t argLen = 0;
        while (argStart+argLen < len && !isspace(src[argStart+argLen])) {
            arg[argLen] = src[argStart+argLen];
            argLen++;
        }
        arg[argLen] = 0;
    }

    // continue normal execution
    if (!strcmp(query, ":c") ||
        !strcmp(query, ":continue")) {
        debug.requestedExit = 1;
        
    } else if (!strcmp(query, ":palette")) {
        mod16Vector_t data[4];    
        int i = atoi(arg);
        if (mod16_native_get_palette_info(
            i,
            data        
        )) {
            debug_println("Palette %d", MOD16Debug_Color__Code, i);
            debug_println("[1] back      -> %g %g %g", MOD16Debug_Color__Code, data[0].x, data[0].y, data[0].z);
            debug_println("[2] mid-back  -> %g %g %g", MOD16Debug_Color__Code, data[1].x, data[1].y, data[1].z);
            debug_println("[3] mid-front -> %g %g %g", MOD16Debug_Color__Code, data[2].x, data[2].y, data[2].z);
            debug_println("[4] front     -> %g %g %g", MOD16Debug_Color__Code, data[3].x, data[3].y, data[3].z);
        } else {
            debug_println(
                "No such palette.",
                MOD16Debug_Color__Error         
            );
        
        }        
        
    // print info on a given sprite
    } else if (!strcmp(query, ":tile")) {
        uint8_t data[64];    
        int i = atoi(arg);
        if (mod16_native_get_tile_info(
            i,
            data        
        )) {
            debug_println("Tile %d", MOD16Debug_Color__Code, i);
            debug_println("%d%d%d%d%d%d%d%d", MOD16Debug_Color__Code, data[0+0], data[0+1], data[0+2], data[0+3], data[0+4], data[0+5], data[0+6], data[0+7]);
            debug_println("%d%d%d%d%d%d%d%d", MOD16Debug_Color__Code, data[8+0], data[8+1], data[8+2], data[8+3], data[8+4], data[8+5], data[8+6], data[8+7]);
            debug_println("%d%d%d%d%d%d%d%d", MOD16Debug_Color__Code, data[16+0], data[16+1], data[16+2], data[16+3], data[16+4], data[16+5], data[16+6], data[16+7]);
            debug_println("%d%d%d%d%d%d%d%d", MOD16Debug_Color__Code, data[24+0], data[24+1], data[24+2], data[24+3], data[24+4], data[24+5], data[24+6], data[24+7]);

            debug_println("%d%d%d%d%d%d%d%d", MOD16Debug_Color__Code, data[32+0], data[32+1], data[32+2], data[32+3], data[32+4], data[32+5], data[32+6], data[32+7]);
            debug_println("%d%d%d%d%d%d%d%d", MOD16Debug_Color__Code, data[40+0], data[40+1], data[40+2], data[40+3], data[40+4], data[40+5], data[40+6], data[40+7]);
            debug_println("%d%d%d%d%d%d%d%d", MOD16Debug_Color__Code, data[48+0], data[48+1], data[48+2], data[48+3], data[48+4], data[48+5], data[48+6], data[48+7]);
            debug_println("%d%d%d%d%d%d%d%d", MOD16Debug_Color__Code, data[56+0], data[56+1], data[56+2], data[56+3], data[56+4], data[56+5], data[56+6], data[56+7]);
        } else {
            debug_println(
                "No such tile.",
                MOD16Debug_Color__Error         
            );
        
        }        
        
    // print info on a given sprite
    } else if (!strcmp(query, ":sprite")) {
        float x, y, rotation,
              scaleX, scaleY,
              centerX, centerY;
        int layer, effect, enabled;
        uint32_t palette, tile;        
        
        int i = atoi(arg);
        if (mod16_native_get_sprite_info(
            i,
            &x, &y, &rotation,
            &scaleX, &scaleY,
            &centerX, &centerY,
            &layer,
            &effect,
            &enabled, 
            
            &palette,
            &tile       
        
        )) {
            debug_println("Sprite    %d", MOD16Debug_Color__Code, i);
            debug_println("enabled : %s", MOD16Debug_Color__Code, enabled ? "true" : "false");
            debug_println("x       : %g", MOD16Debug_Color__Code, x);
            debug_println("y       : %g", MOD16Debug_Color__Code, y);
            debug_println("layer   : %d", MOD16Debug_Color__Code, layer);
            debug_println("effect  : %d", MOD16Debug_Color__Code, effect);
            debug_println("palette : %d", MOD16Debug_Color__Code, palette);
            debug_println("tile    : %d", MOD16Debug_Color__Code, tile);
            debug_println("_________", MOD16Debug_Color__Code);

            debug_println("rotation: %g", MOD16Debug_Color__Code, rotation);
            debug_println("scaleX  : %g", MOD16Debug_Color__Code, scaleX);
            debug_println("scaleY  : %g", MOD16Debug_Color__Code, scaleY);
            debug_println("centerX : %g", MOD16Debug_Color__Code, centerX);
            debug_println("centerY : %g", MOD16Debug_Color__Code, centerY);
        } else {
            debug_println(
                "No such sprite.",
                MOD16Debug_Color__Error         
            );
        
        }
    
    // print callstack
    } else if (!strcmp(query, ":bt") ||
        !strcmp(query, ":backtrace")) {
        
        mod16_matte_backtrace();

    // up the callstack
    } else if (!strcmp(query, ":up") ||
        !strcmp(query, ":u")) {
        debug.callstackLevel += 1;

        if (debug.callstackLevel >= debug.callstackLimit)
            debug.callstackLevel = debug.callstackLimit-1;
        debug_clear();
        mod16_matte_debug_dump();            

    // down the callstack
    } else if (!strcmp(query, ":down") ||
        !strcmp(query, ":d")) {
        if (debug.callstackLevel)
            debug.callstackLevel -= 1;
        debug_clear();

        mod16_matte_debug_dump();            
    } else if (!strcmp(query, ":?") ||
        !strcmp(query, ":help")) {
        
        debug_println("Commands:", MOD16Debug_Color__Code);
        debug_println("", MOD16Debug_Color__Code);
        debug_println(" :up       - up callstack",    MOD16Debug_Color__Code);
        debug_println(" :down     - down callstack",  MOD16Debug_Color__Code);
        debug_println(" :continue - continue exec.",  MOD16Debug_Color__Code);
        debug_println(" :backtrace- print callstack", MOD16Debug_Color__Code);
        debug_println(" :help     - prints this",     MOD16Debug_Color__Code);
        debug_println("", MOD16Debug_Color__Code);
        debug_println("Otherwise, runs any expression", MOD16Debug_Color__Code);
        debug_println("in the current scope", MOD16Debug_Color__Code);
        
        

    } else {


        matteString_t * src = matte_string_create();
        matte_string_concat_printf(src, "return import(module:'Matte.Core.Introspect')(value:%s);", query);


        matteValue_t result = matte_vm_run_scoped_debug_source(
            debug.vm,
            src,
            debug.callstackLevel + (matte_vm_get_stackframe_size(debug.vm) - debug.callstackLimit),
            mod16_matte_query__error,
            NULL
        );   
        
        if (result.binID == MATTE_VALUE_TYPE_STRING) {
            const matteString_t * content = matte_value_string_get_string_unsafe(debug.heap, result);
            matteString_t * working = matte_string_create();

            int i;
            int len = matte_string_get_length(content);
            for(i = 0; i < len; ++i) {
                if (matte_string_get_char(content, i) == '\n') { 
                    debug_println("%s", MOD16Debug_Color__Code, matte_string_get_c_str(working));
                    matte_string_clear(working);
                } else {
                    matte_string_append_char(working, matte_string_get_char(content, i));
                }
            }
            if (matte_string_get_length(working))
                debug_println("%s", MOD16Debug_Color__Code, matte_string_get_c_str(working));
            matte_string_destroy(working);
            
        } else {
            debug_println("  (invalid value)", MOD16Debug_Color__Error);
        }
    }
    debug_show_text();
    free(query);   
    free(arg); 
    return matte_heap_new_value(debug.heap);
    
    
}




static matteValue_t mod16_native__debug_context_is_allowed(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    matteValue_t out = matte_heap_new_value(heap);
    
    matte_value_into_boolean(heap, &out, debug.enabled);
    return out;
}




void mod16_debug_init(matte_t * m, int enabled, const char * romPath) {
    matteVM_t * vm = matte_get_vm(m);

    debug.enabled = enabled;   
    debug.active = 0;
    debug.heap = matte_vm_get_heap(vm);
    debug.vm = vm;
    debug.matte = m;
    debug.files = matte_table_create_hash_matte_string();
    debug.promptConsole = matte_string_create();
    
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
        matte_vm_set_unhandled_callback(vm, mod16_debug_unhandled_error, NULL);
        matte_vm_set_debug_callback(vm, mod16_matte_debug_event, NULL);
    }
    
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__debug_context_bind"),       5, mod16_native__debug_context_bind, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__debug_context_is_allowed"), 0, mod16_native__debug_context_is_allowed, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__debug_context_enter"),      0, mod16_native__debug_context_enter, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__debug_context_query"),      1, mod16_native__debug_context_query, NULL);
        
}

