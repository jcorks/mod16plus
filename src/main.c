#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "matte/src/matte_vm.h"
#include "matte/src/matte.h"
#include "matte/src/matte_string.h"
#include "matte/src/matte_bytecode_stub.h"

#include "rom.h"
#include "dump.h"
#include "package.h"
#include "debug.h"
#include "native.h"
// host of external functions that are required to be implemented so that the 
// behavior of the engine is met.



#include "api/api_rom"
//#include "debug/debug_rom"
static int IS_DEBUG = 0;
static uint32_t mod16_native__import(
    matteVM_t * vm,
    const matteString_t * importPath,
    const matteString_t * alias,
    void * m
) {
    // Import is ONLY used for MOD16.Core
    // 
    if (matte_string_test_eq(importPath, MATTE_VM_STR_CAST(vm, "Mod16Plus.Core"))) {
        uint32_t dataLength;
        uint32_t id = matte_vm_get_new_file_id(vm, importPath);


        char * realSrc = malloc(API_ROM_SIZE+1);
        memcpy(realSrc, API_ROM_DATA, API_ROM_SIZE);
        realSrc[API_ROM_SIZE] = 0;            
        matteString_t * err = matte_string_create();
        uint8_t * data = matte_compile_source(m, &dataLength, realSrc, err);
        
        if (matte_string_get_length(err)) {
            printf("Error compiling core: %s\n", matte_string_get_c_str(err));
            return 0;
        }
        matte_vm_add_stubs(
            vm,
            matte_bytecode_stubs_from_bytecode(
                matte_vm_get_store(vm),
                id,
                data, 
                dataLength 
            )
        );
        free(realSrc);
        return id;
    }
    /*
    else if (IS_DEBUG && matte_string_test_eq(importPath, MATTE_VM_STR_CAST(vm, "MOD16.Debug"))) {
        *dataLength = DEBUG_ROM_SIZE;
        *preexistingFileID = matte_vm_get_new_file_id(vm, importPath);
        uint8_t * out = malloc(DEBUG_ROM_SIZE);
        memcpy(out, DEBUG_ROM_DATA, DEBUG_ROM_SIZE);
        return out;       
    }
    */
    
 
    return 0;
}




static void mod16_native__print(matteVM_t * vm, const matteString_t * str, void * ud) {
    printf("%s\n", matte_string_get_c_str(str));
}



int main(int argc, char ** argv) {
    int developRom = 0;;
    char * devPath = NULL;
    if (argc < 3||
            (
                strcmp(argv[1], "run") &&
                strcmp(argv[1], "debug") &&
                strcmp(argv[1], "package")
            )
        ) {
        printf("Usage:\n   %s run [path to ROM file]\n   %s debug [path to ROM file]\n   %s package [path to package.json]\n\n",
            argv[0],        
            argv[0],
            argv[0]
        );
        return 0;
    }
    // its annoying to get that all the time when packaging 
    // treat this mode like a compilation mode
    if (strcmp(argv[1], "package"))
        printf("Mod16+\nJohnathan Corkery, 2022\njcorkery@umich.edu\n\n");
       


    // choose the source of the rom. For both run and debug, the rom is external.
    // For develop, the UI is a preset ROM.
    const uint8_t * romBytes = NULL;
    uint32_t romLength = 0;
    if (!strcmp(argv[1], "package")) {    
        return mod16_package(argv[2]);
        
    } else {
        romBytes = dump_bytes(argv[2], &romLength);
        
    }
    
    if (argc > 3) {
        devPath = argv[3];
    }

    
    if (romLength == 0) {
        printf("The ROM was empty or unreadable. Exiting.\n");
        return 1;
    }
    matte_t * m = matte_create();
    matteVM_t * vm = matte_get_vm(m);
    matte_vm_set_print_callback(vm, mod16_native__print, NULL);
    

    IS_DEBUG = !strcmp(argv[1], "debug");
    mod16_debug_init(m, IS_DEBUG, argv[2]);







    // dump rom to memory and hook import
    mod16ROM_UnpackError_t result = MOD16_ROM_UNPACK_ERROR__SIZE_MISMATCH;
    mod16ROM_t * rom = mod16_rom_unpack(romBytes, romLength, &result); 
    if (result != 0) {
        printf("Unpacking ROM resulted in error:\n");
        switch(result) {
          case MOD16_ROM_UNPACK_ERROR__TOO_SMALL:
            printf("The ROM is too small to be valid.\n");
            break;
            
          case MOD16_ROM_UNPACK_ERROR__BAD_HEADER:
            printf("The header is incorrect. This can happen if the ROM source is corrupted at the start or is not a ROM file.\n");
            break;
          
            
          case MOD16_ROM_UNPACK_ERROR__UNSUPPORTED_VERSION:
            printf("The ROM version is unsupported.\n");
            break;
            
            // the ROM data has an invalid size of some kind,
            // likely indicating a corrupted ROM.
          case MOD16_ROM_UNPACK_ERROR__SIZE_MISMATCH:
            printf("The ROM has inconsistencies and is unreadable, likely due to corruption.\n");
            
        }
        return 1;
    }

    // tell the implementing backend to process the rom.
    mod16_native_commit_rom(rom, m);
    
    
    // next link up import
    matte_vm_set_import(
        vm,
        mod16_native__import,
        m
    );
    
    // enable extra features needed for development 
    mod16_package_bind_natives(vm, devPath);

    
    // ALWAYS import the special scripts before 
    // the main (for security purposes)
    matte_vm_import(
        vm,
        MATTE_VM_STR_CAST(vm, "Mod16Plus.Core"),
        MATTE_VM_STR_CAST(vm, "Mod16Plus.Core"),
        0,
        matte_store_new_value(matte_vm_get_store(vm))
    );    


    
    
    
    // begin the loop
    return mod16_native_main_loop(m);
}
