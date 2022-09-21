#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "matte/src/matte_vm.h"
#include "matte/src/matte.h"
#include "matte/src/matte_string.h"

#include "develop/develop.h"
#include "rom.h"
#include "dump.h"
#include "package.h"

// host of external functions that are required to be implemented so that the 
// behavior of the engine is met.

extern void ses_native__commit_rom();
extern int ses_native__main_loop(matte_t *);

extern matteValue_t ses_native__sprite_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
extern matteValue_t ses_native__engine_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
extern matteValue_t ses_native__palette_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
extern matteValue_t ses_native__tile_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
extern matteValue_t ses_native__input_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
extern matteValue_t ses_native__audio_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
extern matteValue_t ses_native__bg_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);

extern matteValue_t ses_native__palette_query(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
extern matteValue_t ses_native__tile_query(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);




#include "api/api_rom"
static uint8_t * ses_native__import(
    matteVM_t * vm,
    const matteString_t * importPath,
    uint32_t * preexistingFileID,
    uint32_t * dataLength,
    void * usrdata
) {
    // special case
    if (matte_string_test_eq(importPath, MATTE_VM_STR_CAST(vm, "SES.Core"))) {
        *dataLength = API_ROM_SIZE;
        *preexistingFileID = matte_vm_get_new_file_id(vm, importPath);
        uint8_t * out = malloc(API_ROM_SIZE);
        memcpy(out, API_ROM_DATA, API_ROM_SIZE);
        return out;       
    }
    
    // else, linear search for proper name.
    uint32_t i;
    matteString_t * str = matte_string_create();
    for(i = 0; i < ses_rom_get_bytecode_segment_count(); ++i) {
        const uint8_t * data = ses_rom_get_bytecode_segment(
            i, 
            dataLength,
            str
        );
        
        if (matte_string_test_eq(importPath, str)) {
            *preexistingFileID = matte_vm_get_new_file_id(vm, str);
            uint8_t * out = malloc(*dataLength);
            memcpy(out, data, *dataLength);
            return out;
        }
    }   
    *dataLength = 0;
    return NULL;
}


static void ses_native__unhandled_error(
    matteVM_t * vm, 
    uint32_t file, 
    int lineNumber, 
    matteValue_t val, 
    void * userdata
) {
    if (val.binID == MATTE_VALUE_TYPE_OBJECT) {
        matteValue_t s = matte_value_object_access_string(matte_vm_get_heap(vm), val, MATTE_VM_STR_CAST(vm, "summary"));
        if (s.binID) {
            
            printf(
                "Unhandled error: %s\n", 
                matte_string_get_c_str(matte_value_string_get_string_unsafe(matte_vm_get_heap(vm), s))
            );
            fflush(stdout);
            return;
        }
    }
    
    printf(
        "Unhandled error (%s, line %d)\n", 
        matte_string_get_c_str(matte_vm_get_script_name_by_id(vm, file)), 
        lineNumber
    );
}

static void ses_native__print(matteVM_t * vm, const matteString_t * str, void * ud) {
    printf("%s\n", matte_string_get_c_str(str));
}



int main(int argc, char ** argv) {
    printf("Sprite Entertainment System\nJohnathan Corkery, 2022\njcorkery@umich.edu\n\n");
    
    if (argc < 3||
            (
                strcmp(argv[1], "run") &&
                strcmp(argv[1], "debug") &&
                strcmp(argv[1], "develop") &&
                strcmp(argv[1], "package")
            )
        ) {
        printf("Usage:\n   %s run [path to ROM file]\n   %s debug [path to ROM file]\n   %s develop\n   %s package [path to package.json]\n\n",
            argv[0],        
            argv[0],        
            argv[0],
            argv[0]
        );
        return 0;
    }
    
    


    // choose the source of the rom. For both run and debug, the rom is external.
    // For develop, the UI is a preset ROM.
    const uint8_t * romBytes = NULL;
    uint32_t romLength = 0;
    if (!strcmp(argv[1], "package")) {
        return ses_package(argv[2]);
        
    } else if (!strcmp(argv[1], "develop")) {
        romBytes = ses_develop_get_rom(&romLength);
    } else {
        romBytes = dump_bytes(argv[2], &romLength);
    }
    
    if (romLength == 0) {
        printf("The ROM was empty or unreadable. Exiting.\n");
        return 1;
    }

    matte_t * m = matte_create();
    matteVM_t * vm = matte_get_vm(m);
    matte_vm_set_print_callback(vm, ses_native__print, NULL);
    matte_vm_set_unhandled_callback(vm, ses_native__unhandled_error, NULL);
    

    // all 3 modes require activating the core features.
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__sprite_attrib"), 3, ses_native__sprite_attrib, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__engine_attrib"), 3, ses_native__engine_attrib, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__palette_attrib"), 5, ses_native__palette_attrib, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__tile_attrib"), 3, ses_native__tile_attrib, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__input_attrib"), 3, ses_native__input_attrib, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__audio_attrib"), 4, ses_native__audio_attrib, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__bg_attrib"), 4, ses_native__bg_attrib, NULL);


    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__palette_query"), 3, ses_native__palette_query, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__tile_query"), 2, ses_native__tile_query, NULL);

    // dump rom to memory and hook import
    int result = ses_unpack_rom(romBytes, romLength); 
    if (result != 0) {
        printf("Unpacking ROM resulted in error:\n");
        switch(result) {
          case SES_UNPACK_ERROR__TOO_SMALL:
            printf("The ROM is too small to be valid.\n");
            break;
            
          case SES_UNPACK_ERROR__BAD_HEADER:
            printf("The header is incorrect. This can happen if the ROM source is corrupted at the start or is not a ROM file.\n");
            break;
          
            
          case SES_UNPACK_ERROR__UNSUPPORTED_VERSION:
            printf("The ROM version is unsupported.\n");
            break;
            
            // the ROM data has an invalid size of some kind,
            // likely indicating a corrupted ROM.
          case SES_UNPACK_ERROR__SIZE_MISMATCH:
            printf("The ROM has inconsistencies and is unreadable, likely due to corruption.\n");
            
        }
        return 1;
    }

    // tell the implementing backend to 
    ses_native__commit_rom();
    
    
    // next link up import
    matte_vm_set_import(
        vm,
        ses_native__import,
        NULL
    );
    
    
    // run main.mt
    matteValue_t output = matte_vm_import(
        vm,
        MATTE_VM_STR_CAST(vm, "main.mt"),
        matte_heap_new_value(matte_vm_get_heap(vm))
    );
    
    // begin the loop
    return ses_native__main_loop(m);
}
