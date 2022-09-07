#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "matte/src/matte_vm.h"
#include "matte/src/matte.h"
#include "matte/src/matte_string.h"

#include "develop/develop.h"
#include "rompack.h"

// host of external functions that are required to be implemented so that the 
// behavior of the engine is met.

extern matteValue_t ses_native__sprite_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
extern matteValue_t ses_native__engine_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
extern matteValue_t ses_native__palette_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
extern matteValue_t ses_native__tile_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
extern matteValue_t ses_native__input_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
extern matteValue_t ses_native__audio_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
extern matteValue_t ses_native__bg_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);

extern matteValue_t ses_native__palette_query(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
extern matteValue_t ses_native__tile_query(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
extern matteValue_t ses_native__bg_query(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);


static void * dump_bytes(const char * filename, uint32_t * len) {
    FILE * f = fopen(filename, "rb");
    if (!f) {
        printf("Could not open input file %s\n", filename);
        exit(1);    
    }
    char chunk[2048];
    int chunkSize;
    *len = 0;    
    while(chunkSize = (fread(chunk, 1, 2048, f))) *len += chunkSize;
    fseek(f, 0, SEEK_SET);


    void * out = malloc(*len);
    uint32_t iter = 0;
    while(chunkSize = (fread(chunk, 1, 2048, f))) {
        memcpy(out+iter, chunk, chunkSize);
        iter += chunkSize;
    }
    fclose(f);
    return out;
}

static uint8_t * ses_native__import(
    matteVM_t * vm,
    const matteString_t * importPath,
    uint32_t * preexistingFileID,
    uint32_t * dataLength,
    void * usrdata
) {
    
}




int main(int argc, char ** argv) {
    printf("Sprite Entertaininment System\nJohnathan Corkery, 2022\njcorkery@umich.edu\n\n");
    
    if (argc != 3||
            (
                strcmp(argv[1], "run") &&
                strcmp(argv[1], "debug") &&
                strcmp(argv[1], "develop")
            )
        ) {
        printf("Usage:\n   %s run [path to ROM file]\n   %s debug [path to ROM file]\n   %s develop\n\n",
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
    if (!strcmp(argv[1], "develop")) {
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
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__bg_query"), 3, ses_native__bg_query, NULL);


    // dump rom to memory and hook import
    ses_unpack_rom(romBytes, romLength);
    
    
    // next link up import
    
    // embed api.mt 
    
    // run main.mt
    
    return 0;

}
