#ifndef SES_NATIVE_INCLUDED
#define SES_NATIVE_INCLUDED

#include "matte/src/matte_vm.h"
#include "matte/src/matte_heap.h"
#include "linear.h"


void ses_native_commit_rom();
int ses_native_main_loop(matte_t *);
// normally called for you
// returns whether to continue;
int ses_native_update(matte_t * m);

// swaps between the main and debug contexts
void ses_native_swap_context();




// gets current sprite information of the main context
int ses_native_get_sprite_info(
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
int ses_native_get_tile_info(
    uint32_t tile,
    uint8_t * data
);


// Gets the referre-to palette.
// Returns if the palette exists / was retrievable
// "data" should hold at least 4 values for the 
// 4 slots of color data.
int ses_native_get_palette_info(
    uint32_t tile,
    sesVector_t * data
);

#endif
