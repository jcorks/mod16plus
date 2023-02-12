#include "package.h"
#include "dump.h"
#include "rom.h"
#include "matte/src/matte_string.h"
#include "matte/src/matte_array.h"
#include "matte/src/matte_compiler.h"
#include "matte/src/matte_heap.h"
#include "matte/src/matte_vm.h"
#include "matte/src/matte.h"
#include "matte/src/matte_bytecode_stub.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


char * BASE_DIR = NULL;

#ifdef __unix__
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <dirent.h>
    const char * DIRECTORY_SEPARATOR = "/";
    
    static void assert_base_dir() {
        if (BASE_DIR) return;
        
        BASE_DIR = malloc(1024 + 128);
        getcwd(BASE_DIR, 1024);
        strcat(BASE_DIR, "/MOD16PLUS_DATA/");
        mkdir(BASE_DIR, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }   
    

    



#elif __WIN32__
    #include <windows.h>
    const char * DIRECTORY_SEPARATOR = "\\";

    static void assert_base_dir() {
        if (BASE_DIR) return;
        
        BASE_DIR = malloc(1024, 128);
        _getcwd(BASE_DIR, 1024);
        strcat(BASE_DIR, "\\MOD16PLUS_DATA\\");
        CreateDirectoryA(matte_string_get_c_str(BASE_DIR), NULL);
    }   




#endif


static int is_name_allowed(const matteString_t * file) {
    uint32_t i;
    uint32_t len = matte_string_get_length(file);
    matteString_t * forbidden = matte_string_create_from_c_str("..");
    int initial = matte_string_test_contains(file, forbidden);
    matte_string_destroy(forbidden);
    
    if (initial) return 0;
    for(i = 0; i < len; ++i) {
        if (matte_string_get_char(file, i) == '\\' ||
            matte_string_get_char(file, i) == '/'  ||
            matte_string_get_char(file, i) == '$'  ||
            matte_string_get_char(file, i) == '&'  ||
            matte_string_get_char(file, i) == '#'  ||
            matte_string_get_char(file, i) == '&'  ||
            matte_string_get_char(file, i) == '@'  ||
            matte_string_get_char(file, i) == '?'  ||
            matte_string_get_char(file, i) == '!'  ||
            matte_string_get_char(file, i) == '~'  ||
            matte_string_get_char(file, i) == ','  ||
            matte_string_get_char(file, i) == '*')
            return 0;
    }
    return 1;

}

static matteString_t * build_path(const matteString_t * file) {
    assert_base_dir();
    matteString_t * out = matte_string_create_from_c_str("%s", BASE_DIR);
    matte_string_concat(out, file);
    return out;
}






static matteValue_t package_native__save_source (matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    const matteString_t * name = matte_value_string_get_string_unsafe(heap, args[0]);
    if (!is_name_allowed(name)) {
        matte_vm_raise_error_string(vm, MATTE_VM_STR_CAST(vm, "Name for file not allowed."));
        return matte_heap_new_value(heap);
    }
    matteString_t * fullpath = build_path(name);
    
    // make a byte array
    const matteString_t * data = matte_value_string_get_string_unsafe(heap, args[1]);
    
    
    int result = dump_file(
        matte_string_get_c_str(fullpath),
        (void*)matte_string_get_c_str(data),
        strlen(matte_string_get_c_str(data))
    );
    
    matteValue_t out = matte_heap_new_value(heap);
    matte_value_into_boolean(heap, &out, result);
    return out;

}
static matteValue_t package_native__open_source (matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    if (args[0].binID != MATTE_VALUE_TYPE_STRING)
        return matte_heap_new_value(heap);
    const matteString_t * name = matte_value_string_get_string_unsafe(heap, args[0]);    

    if (!is_name_allowed(name)) {
        matte_vm_raise_error_string(vm, MATTE_VM_STR_CAST(vm, "Name for file not allowed."));
        return matte_heap_new_value(heap);
    }

    matteString_t * fullpath = build_path(name);
    // make a byte array
    
    uint32_t len = 0;
    void * bytes = dump_bytes(
        matte_string_get_c_str(fullpath),
        &len
    );
    
    if (!bytes) {
        matte_string_destroy(fullpath);
        return matte_heap_new_value(heap);
    }
    
    char * str = malloc(len+1);
    memcpy(str, bytes, len);
    str[len] = 0;
    free(bytes);
    
    matteString_t * strval = matte_string_create_from_c_str("%s", str);
    free(str);
    
    
    matteValue_t out = matte_heap_new_value(heap);
    matte_value_into_string(heap, &out, strval);
    matte_string_destroy(strval);
    return out;
}


void mod16_package_bind_natives(matteVM_t * vm, const char * devPath) {

    if (devPath) {
        BASE_DIR = malloc(strlen(devPath)+ 10);
        BASE_DIR[0] = 0;
        strcat(BASE_DIR, devPath);
        strcat(BASE_DIR, DIRECTORY_SEPARATOR);
    }

    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "package_native__save_source"), 2, package_native__save_source, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "package_native__open_source"), 1, package_native__open_source, NULL);
    
}




// given bytes to a text file, returns an array of matteString_t * of 
// each line.
static matteArray_t * package_split(const char * dir, const char * sub) {
    uint8_t * bytes;
    uint32_t len;

    matteString_t * sourceStr = matte_string_create_from_c_str("%s/%s", dir, sub);    
    bytes = dump_bytes(matte_string_get_c_str(sourceStr), &len);
    matte_string_destroy(sourceStr);

    char * str = malloc(len+1);
    memcpy(str, bytes, len);
    str[len] = 0;
    
    matteString_t * text = matte_string_create_from_c_str("%s", str);
    matteString_t * line = matte_string_create();
    matteArray_t * arr = matte_array_create(sizeof(matteString_t *));
    free(str);
    
    uint32_t i;
    len = matte_string_get_length(text);
    for(i = 0; i < len; ++i) {
        uint32_t ch = matte_string_get_char(text, i);
        if (ch == '\n') {
            matte_array_push(arr, line);
            line = matte_string_create();        
        }
        matte_string_append_char(line, ch);
    }
    matte_array_push(arr, line);
    free(bytes);    
    return arr;    
}

static const matteString_t * currentCompiled = NULL;
static void mod16_package__compile_error(
    const matteString_t * str,
    uint32_t line, 
    uint32_t ch,
    void * userdata
) {
    printf(":::!!:::\nError in %s, line %d:%d\n\n%s\n:::!!:::\n", currentCompiled == NULL ? "???" : matte_string_get_c_str(currentCompiled), (int)line, (int)ch, matte_string_get_c_str(str));
    exit(1);
}


static void * dump_bytes_relative(const char * dir, const char * name, uint32_t * len) {
    matteString_t * path = matte_string_create_from_c_str("%s/%s", dir, name);
    void * out = dump_bytes(matte_string_get_c_str(path), len);
    matte_string_destroy(path);
    return out;
}

static int is_string_empty(const matteString_t * line) {
    uint32_t len = matte_string_get_length(line);
    if (len == 0) return 1;
    uint32_t i;
    
    for(i = 0; i < len; ++i) {
        if (matte_string_get_char(line, i) != ' ' &&
            matte_string_get_char(line, i) != '\n' &&
            matte_string_get_char(line, i) != '\t')
            return 0;
    }
    return 1;
}


// will leak memory until finalized

// runs the core json parser on the package.json within dir.
// returns empty on failure.
matteValue_t mod16_package_get_json(matte_t * m, const char * dir) {
    matteVM_t * vm = matte_get_vm(m);
    matteHeap_t * heap = matte_vm_get_heap(vm);
    
    const char * json_unpack_src =
        "return import(module:'Matte.Core.JSON').decode(string:parameters.json);"
    ;
     
    uint32_t bytecodeLen;   
    uint8_t * bytecode = matte_compiler_run(
        json_unpack_src,
        strlen(json_unpack_src),
        &bytecodeLen,
        
        NULL,
        NULL
    );

    
    
    if (!bytecode || !bytecodeLen)
        return matte_heap_new_value(heap);
        
    
    uint32_t fileID = matte_vm_get_new_file_id(vm, MATTE_VM_STR_CAST(vm, "JSONPARSE"));
    matteArray_t * stubs = matte_bytecode_stubs_from_bytecode(
        heap,
        fileID,
        bytecode,
        bytecodeLen
    );
    
    matte_vm_add_stubs(vm, stubs);
        
    

    matteString_t * fullpath = matte_string_create_from_c_str("%s/%s", dir, "cartridge.json");
    uint32_t srclen = 0;
    uint8_t * srcbytes = dump_bytes(matte_string_get_c_str(fullpath), &srclen);
    if (srclen == 0 || srcbytes == NULL)
        return matte_heap_new_value(heap);
    
    uint8_t * srcstring = malloc(srclen+1);
    memcpy(srcstring, srcbytes, srclen);
    srcstring[srclen] = 0;
    free(srcbytes);
    
    matteString_t * srcstringMatte = matte_string_create_from_c_str("%s", srcstring);
    free(srcstring);
    matteValue_t srcval = matte_heap_new_value(heap);
    matte_value_into_string(heap, &srcval, srcstringMatte);
    matte_string_destroy(srcstringMatte);
    
    
    matteValue_t parameters = matte_heap_new_value(heap);
    matte_value_into_new_object_ref(heap, &parameters);
    matteValue_t key = matte_heap_new_value(heap);
    matte_value_into_string(heap, &key, MATTE_VM_STR_CAST(vm, "json"));

    matte_value_object_set(heap, parameters, key, srcval, 1);
    
    return matte_vm_run_fileid(
        vm,
        fileID,
        parameters,
        NULL
    );
}


void mod16_package_debug_callback(
    matteVM_t * vm, 
    matteVMDebugEvent_t event, 
    uint32_t file, 
    int lineNumber, 
    matteValue_t value, 
    void *  userdata
) {
    if (event == MATTE_VM_DEBUG_EVENT__ERROR_RAISED) {
        printf(
            "ERROR: %s\n",
            matte_string_get_c_str(matte_value_string_get_string_unsafe(matte_vm_get_heap(vm), matte_value_as_string(matte_vm_get_heap(vm), value)))            
        );
    }
}


int mod16_package(const char * dir) {
    matte_t * m = matte_create();
    matteVM_t * vm = matte_get_vm(m);
    matteHeap_t * heap = matte_vm_get_heap(vm);
    
    
    matte_vm_set_debug_callback(vm, mod16_package_debug_callback, NULL);
    
    
    matteValue_t json = mod16_package_get_json(m, dir);
    if (json.binID == 0) return 1;
    int out = 0;
    // for each source, dump
    uint32_t len;
    uint8_t * bytes;
    
    matteArray_t * waveformSizes = matte_array_create(sizeof(uint32_t));
    matteArray_t * waveforms = matte_array_create(sizeof(uint8_t*));     
    
    matteArray_t * tileIDs = matte_array_create(sizeof(uint32_t));
    matteArray_t * tiles = matte_array_create(sizeof(uint8_t)*64); // uint8_t *, see mod16_rom_get_tile


    matteArray_t * paletteIDs = matte_array_create(sizeof(uint32_t));
    matteArray_t * palettes = matte_array_create(sizeof(float)*15); // uint8_t *, see mod16_rom_get_palette
    
    
    matteArray_t * bytecodeSegmentNames = matte_array_create(sizeof(matteString_t*));
    matteArray_t * bytecodeSegmentSizes = matte_array_create(sizeof(uint32_t));
    matteArray_t * bytecodeSegments = matte_array_create(sizeof(uint8_t*));
    
    
    matteArray_t * subcartridgeNames = matte_array_create(sizeof(matteString_t*));
    matteArray_t * subcartridgeROMSizes = matte_array_create(sizeof(uint32_t));
    matteArray_t * subcartridgeROMSegments = matte_array_create(sizeof(uint8_t*));
    
    
    uint32_t i;
    
    
    
    // waveforms 
    matteValue_t next = matte_value_object_access_string(heap, json, MATTE_VM_STR_CAST(vm, "waveforms"));
    if (next.binID == MATTE_VALUE_TYPE_OBJECT) {
        len = matte_value_object_get_number_key_count(heap, next);
        for(i = 0; i < len; ++i) {
            matteValue_t name = matte_value_object_access_index(heap, next, i);
            if (name.binID != MATTE_VALUE_TYPE_STRING) {
                printf("Error on waveform %d: value is not a string.\n", i+1);
                goto L_FAIL;
            }

            uint32_t byteLen;
            bytes = dump_bytes_relative(
                dir,
                matte_string_get_c_str(matte_value_string_get_string_unsafe(heap, name)), 
                &byteLen
            );
            
            // raw waveforms (for now)
            // todo: possibly ogg
            matte_array_push(waveforms, bytes);
            matte_array_push(waveformSizes, byteLen);
        }        
    }
    

    // tiles 
    next = matte_value_object_access_string(heap, json, MATTE_VM_STR_CAST(vm, "tiles"));
    if (next.binID == MATTE_VALUE_TYPE_OBJECT) {
        len = matte_value_object_get_number_key_count(heap, next);
        for(i = 0; i < len; ++i) {
            matteValue_t set = matte_value_object_access_index(heap, next, i);
            if (set.binID != MATTE_VALUE_TYPE_OBJECT) {
                printf("Error on tile %d: value is not an array.\n", i+1);
                goto L_FAIL;
            }
            
            matteValue_t id   = matte_value_object_access_index(heap, set, 0);
            matteValue_t path = matte_value_object_access_index(heap, set, 1);

            if (id.binID != MATTE_VALUE_TYPE_NUMBER) {
                printf("Error on tile %d: first array value is not a number.\n", i+1);
                goto L_FAIL;
            }

            if (path.binID != MATTE_VALUE_TYPE_STRING) {
                printf("Error on tile %d: second array value is not a string.\n", i+1);
                goto L_FAIL;
            }
            
            
            
            uint32_t byteLen;
            bytes = dump_bytes_relative(
                dir,
                matte_string_get_c_str(matte_value_string_get_string_unsafe(heap, path)), 
                &byteLen
            );

            char paletteBytes[64];
            if (sscanf(bytes, 
                    " %c %c %c %c %c %c %c %c"            
                    " %c %c %c %c %c %c %c %c"            
                    " %c %c %c %c %c %c %c %c"            
                    " %c %c %c %c %c %c %c %c"            

                    " %c %c %c %c %c %c %c %c"            
                    " %c %c %c %c %c %c %c %c"            
                    " %c %c %c %c %c %c %c %c"            
                    " %c %c %c %c %c %c %c %c"            
                , 
                paletteBytes,    paletteBytes+1,  paletteBytes+2,  paletteBytes+3,  paletteBytes+4,  paletteBytes+5,  paletteBytes+6, paletteBytes+7,
                paletteBytes+8,  paletteBytes+9,  paletteBytes+10, paletteBytes+11, paletteBytes+12, paletteBytes+13, paletteBytes+14, paletteBytes+15,
                paletteBytes+16, paletteBytes+17, paletteBytes+18, paletteBytes+19, paletteBytes+20, paletteBytes+21, paletteBytes+22, paletteBytes+23,
                paletteBytes+24, paletteBytes+25, paletteBytes+26, paletteBytes+27, paletteBytes+28, paletteBytes+29, paletteBytes+30, paletteBytes+31,

                paletteBytes+32, paletteBytes+33, paletteBytes+34, paletteBytes+35, paletteBytes+36, paletteBytes+37, paletteBytes+38, paletteBytes+39,
                paletteBytes+40, paletteBytes+41, paletteBytes+42, paletteBytes+43, paletteBytes+44, paletteBytes+45, paletteBytes+46, paletteBytes+47,
                paletteBytes+48, paletteBytes+49, paletteBytes+50, paletteBytes+51, paletteBytes+52, paletteBytes+53, paletteBytes+54, paletteBytes+55,
                paletteBytes+56, paletteBytes+57, paletteBytes+58, paletteBytes+59, paletteBytes+60, paletteBytes+61, paletteBytes+62, paletteBytes+63

                ) != 64) {
                printf("Tile sheet %s is malformed. It should contain 64 byte values in text.\n", matte_string_get_c_str(matte_value_string_get_string_unsafe(heap, path)));
                goto L_FAIL;
            }
            int n;
            for(n = 0; n < 64; ++n) {
                switch(paletteBytes[n]) {
                  case '0': paletteBytes[n] = 0; break;
                  case '1': paletteBytes[n] = 1; break;
                  case '2': paletteBytes[n] = 2; break;
                  case '3': paletteBytes[n] = 3; break;
                  case '4': paletteBytes[n] = 4; break;
                  case '5': paletteBytes[n] = 5; break;
                  default:
                    paletteBytes[n] = 0;
                }
            }


            matte_array_push(tiles, paletteBytes);
            uint32_t idVal = matte_value_as_number(heap, id);
            matte_array_push(tileIDs, idVal);
        }        
    }

    // palettes 
    next = matte_value_object_access_string(heap, json, MATTE_VM_STR_CAST(vm, "palettes"));
    if (next.binID == MATTE_VALUE_TYPE_OBJECT) {
        len = matte_value_object_get_number_key_count(heap, next);
        for(i = 0; i < len; ++i) {
            matteValue_t set = matte_value_object_access_index(heap, next, i);
            if (set.binID != MATTE_VALUE_TYPE_OBJECT) {
                printf("Error on palette %d: value is not an array.\n", i+1);
                goto L_FAIL;
            }
            
            matteValue_t id   = matte_value_object_access_index(heap, set, 0);
            matteValue_t path = matte_value_object_access_index(heap, set, 1);

            if (id.binID != MATTE_VALUE_TYPE_NUMBER) {
                printf("Error on palette %d: first array value is not a number.\n", i+1);
                goto L_FAIL;
            }

            if (path.binID != MATTE_VALUE_TYPE_STRING) {
                printf("Error on palette %d: second array value is not a string.\n", i+1);
                goto L_FAIL;
            }




            uint32_t byteLen;
            bytes = dump_bytes_relative(
                dir,
                matte_string_get_c_str(matte_value_string_get_string_unsafe(heap, path)), 
                &byteLen
            );

            
            float paletteBytes[12];
            if (sscanf(bytes, 
                    "%f %f %f "            
                    "%f %f %f "            
                    "%f %f %f "            
                    "%f %f %f "            
                    "%f %f %f"            
                    , 
                paletteBytes,   paletteBytes+1, paletteBytes+2, 
                paletteBytes+3, paletteBytes+4, paletteBytes+5,
                paletteBytes+6, paletteBytes+7, paletteBytes+8,
                paletteBytes+9, paletteBytes+10,paletteBytes+11,
                paletteBytes+12, paletteBytes+13,paletteBytes+14
                ) != 15) {
                printf("Palette sheet %s is malformed. It should contain 15 decimal values.\n", matte_string_get_c_str(matte_value_string_get_string_unsafe(heap, path)));
                goto L_FAIL;
            }
            
            matte_array_push(palettes, paletteBytes);
            uint32_t idVal = matte_value_as_number(heap, id);
            matte_array_push(paletteIDs, idVal);

        }        
    }    


    
    // bytecode segments
    next = matte_value_object_access_string(heap, json, MATTE_VM_STR_CAST(vm, "sources"));
    if (next.binID == MATTE_VALUE_TYPE_OBJECT) {
        len = matte_value_object_get_number_key_count(heap, next);
        for(i = 0; i < len; ++i) {
            matteValue_t name = matte_value_object_access_index(heap, next, i);
            if (name.binID != MATTE_VALUE_TYPE_STRING) {
                printf("Error on source %d: value is not a string.\n", i+1);
                goto L_FAIL;
            }

            uint32_t byteLen;
            bytes = dump_bytes_relative(
                dir,
                matte_string_get_c_str(matte_value_string_get_string_unsafe(heap, name)),
                &byteLen
            );
            uint8_t * ref = bytes;

            // now that we have a source, compile it
            uint32_t segmentLength;
            currentCompiled = matte_value_string_get_string_unsafe(heap, name);
            bytes = matte_compiler_run(
                bytes,
                byteLen,
                &segmentLength,
                mod16_package__compile_error, 
                NULL
            );
            currentCompiled = NULL;
            free(ref);
            const matteString_t * line = matte_value_string_get_string_unsafe(heap, name);
            
            matte_array_push(bytecodeSegments, bytes);
            matte_array_push(bytecodeSegmentSizes, segmentLength);
            matte_array_push(bytecodeSegmentNames, line);
            
        }
    }
    
    // subcartridges
    next = matte_value_object_access_string(heap, json, MATTE_VM_STR_CAST(vm, "subcartridges"));
    if (next.binID == MATTE_VALUE_TYPE_OBJECT) {
        len = matte_value_object_get_number_key_count(heap, next);
        for(i = 0; i < len; ++i) {
            matteValue_t name = matte_value_object_access_index(heap, next, i);
            if (name.binID != MATTE_VALUE_TYPE_STRING) {
                printf("Error on subcartridge %d: value is not a string.\n", i+1);
                goto L_FAIL;
            }

            uint32_t byteLen;
            bytes = dump_bytes_relative(
                dir,
                matte_string_get_c_str(matte_value_string_get_string_unsafe(heap, name)),
                &byteLen
            );

            const matteString_t * line = matte_value_string_get_string_unsafe(heap, name);

            
            matte_array_push(subcartridgeROMSegments, bytes);
            matte_array_push(subcartridgeROMSizes, byteLen);
            matte_array_push(subcartridgeNames, line);
            
        }
    }    
    
    mod16ROM_t *romV = mod16_rom_create(
        waveformSizes, // uint32_t
        waveforms, // uint8_t *     

        tileIDs,        
        tiles, // uint8_t, see mod16_rom_get_tile

        paletteIDs, 
        palettes, // float, see mod16_rom_get_palette
        
        
        bytecodeSegmentNames, // matteString_t *
        bytecodeSegmentSizes, // uint32_t
        bytecodeSegments, // uint8_t *    
        
        
        subcartridgeNames, // matteString_t *
        subcartridgeROMSizes, // uint32_t
        subcartridgeROMSegments // uint8_t *            
        
    );

    matteArray_t * romBytes = mod16_rom_pack(romV);
    
    matteString_t * outname = matte_string_create_from_c_str("%s/%s", dir, "rom.mod16");    

    out = dump_file(matte_string_get_c_str(outname), matte_array_get_data(romBytes), matte_array_get_size(romBytes));
    
  L_FAIL:;
    matte_array_destroy(waveformSizes);
    matte_array_destroy(waveforms);
    matte_array_destroy(tileIDs);
    matte_array_destroy(tiles);
    matte_array_destroy(paletteIDs);
    matte_array_destroy(palettes);
    matte_array_destroy(bytecodeSegmentNames);
    matte_array_destroy(bytecodeSegmentSizes);
    matte_array_destroy(bytecodeSegments);
    matte_array_destroy(subcartridgeNames);
    matte_array_destroy(subcartridgeROMSizes);
    matte_array_destroy(subcartridgeROMSegments);
    return !out;
}

