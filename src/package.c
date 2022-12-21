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

#ifdef __unix__
    const char * BASE_DIR = "/usr/share/SES/projects/";
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <dirent.h>
    static void make_project_dir(const char * name) {
        matteString_t * path = matte_string_create_from_c_str("%s%s", BASE_DIR, name);
        mkdir(matte_string_get_c_str(path), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        matte_string_destroy(path);
    };
    
    static matteString_t * build_path(const matteString_t * project, const matteString_t * file) {
        matteString_t * out = matte_string_create_from_c_str("%s", BASE_DIR);
        matte_string_concat(out, project);
        matteString_t * slash = matte_string_create_from_c_str("/");
        matte_string_concat(out, slash);
        matte_string_concat(out, file);
        matte_string_destroy(slash);
        return out;
    }

    static matteArray_t * list_projects() {
        matteArray_t * arr = matte_array_create(sizeof(matteString_t *));
        struct dirent * dir;
        DIR * d = opendir(BASE_DIR);
        if (d) {
            while((dir = readdir(d)) != NULL) {
                if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")) continue;
                matteString_t * str = matte_string_create_from_c_str("%s", dir->d_name);
                matte_array_push(arr, str);
            }
        }
        closedir(d);
        return arr;
    }

#elif __WIN32__
    const char * BASE_DIR = "C:\\SES\\projects\\";
    #include <windows.h>
    static void make_project_dir(const char * name) {
        matteString_t * path = matte_string_create_from_c_str("%s%s", BASE_DIR, name);
        CreateDirectoryA(matte_string_get_c_str(path), NULL);
        matte_string_destroy(path);
    };
    
    static matteString_t * build_path(const matteString_t * project, const matteString_t * file) {
        matteString_t * out = matte_string_create_from_c_str("%s", BASE_DIR);
        matte_string_concat(out, project);
        matteString_t * slash = matte_string_create_from_c_str("\\");
        matte_string_concat(out, slash);
        matte_string_concat(out, file);
        matte_string_destroy(slash);
        return out;
    }

    static matteArray_t * list_projects() {
        matteArray_t * arr = matte_array_create(sizeof(matteString_t *));

        WIN32_FIND_DATA data;
        matteString_t * query = matte_string_create_from_c_str("%s\\*", BASE_DIR);

        HANDLE file = FindFirstFile(matte_string_get_c_str(query), &data);
        if (file != INVALID_HANDLE_VALUE) {
            do {
                if (!strcmp(data.cFileName, ".") || !strcmp(data.cFileName, "..")) continue;
                matteString_t * str = matte_string_create_from_c_str("%s", data.cFileName);
                matte_array_push(arr, str);
            } while(FindNextFile(file, &data));
            FindClose(file);
        }
        return arr;
    }



#endif


static matteValue_t package_native__save_source (matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    matteString_t * fullpath = build_path(
        matte_value_string_get_string_unsafe(heap, args[0]),
        matte_value_string_get_string_unsafe(heap, args[1])    
    );
    
    // make a byte array
    const matteString_t * data = matte_value_string_get_string_unsafe(heap, args[2]);
    
    
    int result = dump_file(
        matte_string_get_c_str(fullpath),
        (void*)matte_string_get_c_str(data),
        strlen(matte_string_get_c_str(data))
    );
    
    matteValue_t out = matte_heap_new_value(heap);
    matte_value_into_boolean(heap, &out, result);
    return out;

}
static matteValue_t package_native__make_project(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    make_project_dir(
        matte_string_get_c_str(matte_value_string_get_string_unsafe(heap, args[0]))
    );
    return matte_heap_new_value(heap);

}
static matteValue_t package_native__open_source (matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    if (args[1].binID != MATTE_VALUE_TYPE_STRING)
        return matte_heap_new_value(heap);
    matteString_t * fullpath = build_path(
        matte_value_string_get_string_unsafe(heap, args[0]),
        matte_value_string_get_string_unsafe(heap, args[1])    
    );
    // make a byte array
    const matteString_t * data = matte_value_string_get_string_unsafe(heap, args[2]);
    
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

static matteValue_t package_native__list_projects(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    matteArray_t * a = list_projects();
    uint32_t i;
    uint32_t len = matte_array_get_size(a);

    matteArray_t * vals = matte_array_create(sizeof(matteValue_t));
    for(i = 0; i < len; ++i) {
        matteValue_t d = matte_heap_new_value(heap);
        matte_value_into_string(heap, &d, matte_array_at(a, matteString_t *, i));
        matte_string_destroy(matte_array_at(a, matteString_t *, i));
        matte_array_push(vals, d);
    }
    matte_array_destroy(a);
    matteValue_t out = matte_heap_new_value(heap);
    matte_value_into_new_object_array_ref(heap, &out, vals);
    matte_array_destroy(vals);        
    return out;
}

void ses_package_bind_natives(matteVM_t * vm) {
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "package_native__save_source"), 3, package_native__save_source, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "package_native__make_project"), 2, package_native__make_project, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "package_native__open_source"), 2, package_native__open_source, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "package_native__list_projects"), 0, package_native__list_projects, NULL);
    
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
static void ses_package__compile_error(
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
matteValue_t ses_package_get_json(matte_t * m, const char * dir) {
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
        
    

    matteString_t * fullpath = matte_string_create_from_c_str("%s/%s", dir, "package.json");
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


int ses_package(const char * dir) {
    matte_t * m = matte_create();
    matteVM_t * vm = matte_get_vm(m);
    matteHeap_t * heap = matte_vm_get_heap(vm);
    
    
    matteValue_t json = ses_package_get_json(m, dir);
    if (json.binID == 0) return 0;
    
    // for each source, dump
    uint32_t len;
    uint8_t * bytes;
    
    matteArray_t * waveformSizes = matte_array_create(sizeof(uint32_t));
    matteArray_t * waveforms = matte_array_create(sizeof(uint8_t*));     
    
    matteArray_t * tileIDs = matte_array_create(sizeof(uint32_t));
    matteArray_t * tiles = matte_array_create(sizeof(uint8_t)*64); // uint8_t *, see ses_rom_get_tile


    matteArray_t * paletteIDs = matte_array_create(sizeof(uint32_t));
    matteArray_t * palettes = matte_array_create(sizeof(uint8_t)*12); // uint8_t *, see ses_rom_get_palette
    
    
    matteArray_t * bytecodeSegmentNames = matte_array_create(sizeof(matteString_t*));
    matteArray_t * bytecodeSegmentSizes = matte_array_create(sizeof(uint32_t));
    matteArray_t * bytecodeSegments = matte_array_create(sizeof(uint8_t*));
    
    uint32_t i;
    
    
    
    // waveforms 
    matteValue_t next = matte_value_object_access_string(heap, json, MATTE_VM_STR_CAST(vm, "waveforms"));
    if (next.binID == MATTE_VALUE_TYPE_OBJECT) {
        len = matte_value_object_get_number_key_count(heap, next);
        for(i = 0; i < len; ++i) {
            matteValue_t name = matte_value_object_access_index(heap, next, i);
            if (name.binID != MATTE_VALUE_TYPE_STRING) {
                printf("Error on waveform %d: value is not a string.\n", i+1);
                exit(1);
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
                exit(1);
            }
            
            matteValue_t id   = matte_value_object_access_index(heap, next, 0);
            matteValue_t path = matte_value_object_access_index(heap, next, 1);

            if (id.binID != MATTE_VALUE_TYPE_NUMBER) {
                printf("Error on tile %d: first array value is not a number.\n", i+1);
                exit(1);
            }

            if (path.binID != MATTE_VALUE_TYPE_STRING) {
                printf("Error on tile %d: second array value is not a string.\n", i+1);
                exit(1);
            }
            
            
            
            uint32_t byteLen;
            bytes = dump_bytes_relative(
                dir,
                matte_string_get_c_str(matte_value_string_get_string_unsafe(heap, path)), 
                &byteLen
            );

            if (byteLen % 64 != 0) {
                printf("Tile sheet %s is misaligned and does not contain a multiple of 64 bytes.\n", matte_string_get_c_str(matte_value_string_get_string_unsafe(heap, path)));
                exit(1);
            }

            matte_array_push_n(tiles, bytes, byteLen/64);
            matte_array_push(tileIDs, id);
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
                exit(1);
            }
            
            matteValue_t id   = matte_value_object_access_index(heap, next, 0);
            matteValue_t path = matte_value_object_access_index(heap, next, 1);

            if (id.binID != MATTE_VALUE_TYPE_NUMBER) {
                printf("Error on palette %d: first array value is not a number.\n", i+1);
                exit(1);
            }

            if (path.binID != MATTE_VALUE_TYPE_STRING) {
                printf("Error on palette %d: second array value is not a string.\n", i+1);
                exit(1);
            }




            uint32_t byteLen;
            bytes = dump_bytes_relative(
                dir,
                matte_string_get_c_str(matte_value_string_get_string_unsafe(heap, path)), 
                &byteLen
            );
            if (byteLen % 12 != 0) {
                printf("Palette sheet %s is misaligned and does not contain a multiple of 12 bytes.\n", matte_string_get_c_str(matte_value_string_get_string_unsafe(heap, path)));
                exit(1);
            }
            // raw waveforms (for now)
            // todo: possibly ogg
            matte_array_push_n(palettes, bytes, byteLen/12);
            matte_array_push(paletteIDs, id);

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
                exit(1);
            }

            uint32_t byteLen;
            bytes = dump_bytes_relative(
                dir,
                matte_string_get_c_str(matte_value_string_get_string_unsafe(heap, name)),
                &byteLen
            );

            // now that we have a source, compile it
            uint32_t segmentLength;
            currentCompiled = matte_value_string_get_string_unsafe(heap, name);
            bytes = matte_compiler_run(
                bytes,
                byteLen,
                &segmentLength,
                ses_package__compile_error, 
                NULL
            );
            currentCompiled = NULL;
            
            const matteString_t * line = matte_value_string_get_string_unsafe(heap, name);
            
            matte_array_push(bytecodeSegments, bytes);
            matte_array_push(bytecodeSegmentSizes, segmentLength);
            matte_array_push(bytecodeSegmentNames, line);
            
        }
    }
    
    matteArray_t * romBytes = ses_pack_rom(
        waveformSizes, // uint32_t
        waveforms, // uint8_t *     

        tileIDs,        
        tiles, // uint8_t, see ses_rom_get_tile

        paletteIDs, 
        palettes, // uint8_t, see ses_rom_get_palette
        
        
        bytecodeSegmentNames, // matteString_t *
        bytecodeSegmentSizes, // uint32_t
        bytecodeSegments // uint8_t *    
    );
    
    matteString_t * outname = matte_string_create_from_c_str("%s/%s", dir, "rom.ses");    

    return dump_file(matte_string_get_c_str(outname), matte_array_get_data(romBytes), matte_array_get_size(romBytes));
}

