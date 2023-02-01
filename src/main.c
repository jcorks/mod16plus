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
#include "debug.h"
#include "native.h"
// host of external functions that are required to be implemented so that the 
// behavior of the engine is met.



#include "api/api_rom"
//#include "debug/debug_rom"
static int IS_DEBUG = 0;
static uint8_t * ses_native__import(
    matteVM_t * vm,
    const matteString_t * importPath,
    uint32_t * preexistingFileID,
    uint32_t * dataLength,
    void * rom
) {
    // Import is ONLY used for SES.Core
    // 
    if (matte_string_test_eq(importPath, MATTE_VM_STR_CAST(vm, "SES.Core"))) {
        *dataLength = API_ROM_SIZE;
        *preexistingFileID = matte_vm_get_new_file_id(vm, importPath);
        uint8_t * out = malloc(API_ROM_SIZE);
        memcpy(out, API_ROM_DATA, API_ROM_SIZE);
        return out;       
    } else {
        *dataLength = 0;    
        return NULL;
    }
    /*
    else if (IS_DEBUG && matte_string_test_eq(importPath, MATTE_VM_STR_CAST(vm, "SES.Debug"))) {
        *dataLength = DEBUG_ROM_SIZE;
        *preexistingFileID = matte_vm_get_new_file_id(vm, importPath);
        uint8_t * out = malloc(DEBUG_ROM_SIZE);
        memcpy(out, DEBUG_ROM_DATA, DEBUG_ROM_SIZE);
        return out;       
    }
    */
    
 
    return NULL;
}




static void ses_native__print(matteVM_t * vm, const matteString_t * str, void * ud) {
    printf("%s\n", matte_string_get_c_str(str));
}



int main(int argc, char ** argv) {
    int developRom = 0;;
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
    // its annoying to get that all the time when packaging 
    // treat this mode like a compilation mode
    if (strcmp(argv[1], "package"))
        printf("Sprite Entertainment System\nJohnathan Corkery, 2022\njcorkery@umich.edu\n\n");
    
    


    // choose the source of the rom. For both run and debug, the rom is external.
    // For develop, the UI is a preset ROM.
    const uint8_t * romBytes = NULL;
    uint32_t romLength = 0;
    if (!strcmp(argv[1], "package")) {    
        return ses_package(argv[2]);
        
    } else {
        developRom = (!strcmp(argv[1], "develop"));
        romBytes = dump_bytes(argv[2], &romLength);
    }
    
    if (romLength == 0) {
        printf("The ROM was empty or unreadable. Exiting.\n");
        return 1;
    }
    matte_t * m = matte_create();
    matteVM_t * vm = matte_get_vm(m);
    matte_vm_set_print_callback(vm, ses_native__print, NULL);
    

    IS_DEBUG = !strcmp(argv[1], "debug");
    ses_debug_init(m, IS_DEBUG, argv[2]);







    // dump rom to memory and hook import
    sesROM_UnpackError_t result = SES_ROM_UNPACK_ERROR__SIZE_MISMATCH;
    sesROM_t * rom = ses_rom_unpack(romBytes, romLength, &result); 
    if (result != 0) {
        printf("Unpacking ROM resulted in error:\n");
        switch(result) {
          case SES_ROM_UNPACK_ERROR__TOO_SMALL:
            printf("The ROM is too small to be valid.\n");
            break;
            
          case SES_ROM_UNPACK_ERROR__BAD_HEADER:
            printf("The header is incorrect. This can happen if the ROM source is corrupted at the start or is not a ROM file.\n");
            break;
          
            
          case SES_ROM_UNPACK_ERROR__UNSUPPORTED_VERSION:
            printf("The ROM version is unsupported.\n");
            break;
            
            // the ROM data has an invalid size of some kind,
            // likely indicating a corrupted ROM.
          case SES_ROM_UNPACK_ERROR__SIZE_MISMATCH:
            printf("The ROM has inconsistencies and is unreadable, likely due to corruption.\n");
            
        }
        return 1;
    }

    // tell the implementing backend to process the rom.
    ses_native_commit_rom(rom, m);
    
    
    // next link up import
    matte_vm_set_import(
        vm,
        ses_native__import,
        rom
    );
    
    // enable extra features needed for development 
    ses_package_bind_natives(vm, developRom || IS_DEBUG);

    
    // ALWAYS import the special scripts before 
    // the main (for security purposes)
    matte_vm_import(
        vm,
        MATTE_VM_STR_CAST(vm, "SES.Core"),
        matte_heap_new_value(matte_vm_get_heap(vm))
    );    


    
    
    
    // begin the loop
    return ses_native_main_loop(m);
}
