#ifndef MOD16_NATIVE_INCLUDED
#define MOD16_NATIVE_INCLUDED

#include "matte/src/matte_vm.h"
#include "matte/src/matte_heap.h"
#include "linear.h"
#include "rom.h"


void mod16_native_commit_rom(mod16ROM_t *, matte_t *);
int mod16_native_main_loop(matte_t *);
// normally called for you
// returns whether to continue;
int mod16_native_update(matte_t * m);

// swaps between the main and debug contexts
void mod16_native_swap_context();




// gets current sprite information of the main context
int mod16_native_get_sprite_info(
    uint32_t index,
    
    float * x,
    float * y,
    float * rotation,
    float * scaleX,
    float * scaleY,
    float * centerX,
    float * centerY,
    int * layer,
    int * effect,
    int * enabled,

    uint32_t * palette,
    uint32_t * tile    
    
);


// gets current tile information of the main 
// context. The input data pointer should 
// be able to hold 8x8 bytes (64)
int mod16_native_get_tile_info(
    uint32_t tile,
    uint8_t * data
);


// Gets the referre-to palette.
// Returns if the palette exists / was retrievable
// "data" should hold at least 4 values for the 
// 4 slots of color data.
int mod16_native_get_palette_info(
    uint32_t tile,
    mod16Vector_t * data
);

#endif
