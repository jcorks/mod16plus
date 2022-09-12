#include "matte/src/matte_vm.h"
#include "matte/src/matte.h"
#include "matte/src/matte_array.h"
#include "linear.h"
#include <stdio.h>

#include <SDL2/SDL.h>



typedef struct {
    // SDL window
    SDL_Window    * window;
    
    // SDL GLES context
    SDL_GLContext * ctx;

    
    matteVM_t * vm;

    // user function called every frame    
    matteValue_t updateFunc;
    
    // delay before updating the next frame
    uint32_t frameUpdateDelayMS;
    
    // callback ID for the timer event emitter
    SDL_TimerID frameUpdateID;
    
    // an array of SES_Sprite, representing all accessed 
    // sprites.
    matteArray_t * sprites;
    
    // an array of SES_Palette, representing all accessed 
    // palettes
    matteArray_t * palettes;
} SES_SDL;

static SES_SDL sdl = {};






typedef struct {
    float x;
    float y;
    float rotation;
    float scaleX;
    float scaleY;
    float centerX;
    float centerY;
    int layer;
    int effect;
    int enabled;

    uint32_t palette;
    uint32_t tile;
} SES_Sprite;




typedef struct {
    sesVector_t back;
    sesVector_t midBack;
    sesVector_t midFront;
    sesVector_t front; 

} SES_Palette;
/////// EXTERNAL OPENGL INTERFACE  


// initializes the OpenGL context and window.
extern void ses_sdl_gl_init(SDL_Window ** window, SDL_GLContext ** context);



// Sets a tile ID as the "working tile"
// for either writing or reading.
extern void ses_sdl_gl_bind_tile(uint32_t id);

// Sets a texel / pixel for the currently bound 
// tile ID.
// location should be a number from 0 to 63 (the 
// tile to edit) and value should be the new 
// value to derive the color from the palette, from 0 to 4.
extern void ses_sdl_gl_set_tile_pixel(uint8_t location, uint8_t value);

// Gets the current texel/pixel from the currently bound tile ID.
// location should be a number from 0 to 63
extern uint8_t ses_sdl_gl_get_tile_pixel(uint8_t location);

// Unbinds a tile, committing a tiles new data if edited.
extern void ses_sdl_gl_unbind_tile();

extern void ses_sdl_gl_render_begin();

extern void ses_sdl_gl_render_sprite(
    float x, float y,
    float scaleX, float scaleY,
    float centerX, float centerY,
    float rotation,
    int layer,
    int effect,

    sesVector_t back,
    sesVector_t midBack,
    sesVector_t midFront,
    sesVector_t front,
            
    uint32_t id
);

extern void ses_sdl_gl_render_end();



////////



static void ses_sdl_render() {
    ses_sdl_gl_render_begin();
    // start with backgrounds


    // then do sprites
    uint32_t i;
    uint32_t len = matte_array_get_size(sdl.sprites);
    SES_Sprite * iter = matte_array_get_data(sdl.sprites);
    for(i = 0; i < len; ++i, iter++) {
        if (iter->enabled == 0) continue;
        SES_Palette p = {};
        if (iter->palette < matte_array_get_size(sdl.palettes)) {
            p = matte_array_at(sdl.palettes, SES_Palette, iter->palette);
        }
        
        ses_sdl_gl_render_sprite(
            iter->x, iter->y,
            iter->scaleX, iter->scaleY,
            iter->centerX, iter->centerY,
            iter->rotation,
            iter->layer,
            iter->effect,
            
            p.back,
            p.midBack,
            p.midFront,
            p.front,
            
            iter->tile
        );
    }

    // commit to framebuffer 0    
    ses_sdl_gl_render_end();
}




static uint32_t ses_sdl_emit_frame_event(uint32_t interval, void * param) {
    
    SDL_Event event;
    SDL_UserEvent userevent = {};

    userevent.type = SDL_USEREVENT;
    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);    
    return sdl.frameUpdateDelayMS;
}


typedef enum {
    SESNSA_ENABLE,
    SESNSA_ROTATION,
    SESNSA_SCALEX,
    SESNSA_SCALEY,
    SESNSA_POSITIONX,
    SESNSA_POSITIONY,
    SESNSA_CENTERX,
    SESNSA_CENTERY,
    SESNSA_LAYER,
    SESNSA_TILEINDEX,
    SESNSA_EFFECT,
    SESNSA_PALETTE,
} SESNative_SpriteAttribs;




matteValue_t ses_native__sprite_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);

    uint32_t id = matte_value_as_number(heap, args[0]);
    while(id >= matte_array_get_size(sdl.sprites)) {
        SES_Sprite spr = {};
        matte_array_push(sdl.sprites, spr);
    }
    SES_Sprite * spr = &matte_array_at(sdl.sprites, SES_Sprite, id);
    switch((int)matte_value_as_number(heap, args[1])) {
      case SESNSA_ENABLE:
        spr->enabled = matte_value_as_number(heap, args[2]);
        break;
        
      case SESNSA_ROTATION:
        spr->rotation = matte_value_as_number(heap, args[2]);
        break;

      case SESNSA_SCALEX:
        spr->scaleX = matte_value_as_number(heap, args[2]);
        break;

      case SESNSA_SCALEY:
        spr->scaleY = matte_value_as_number(heap, args[2]);
        break;

      case SESNSA_POSITIONX:
        spr->x = matte_value_as_number(heap, args[2]);
        break;
      
      case SESNSA_POSITIONY:
        spr->y = matte_value_as_number(heap, args[2]);
        break;

      case SESNSA_CENTERX:
        spr->centerX = matte_value_as_number(heap, args[2]);
        break;

      case SESNSA_CENTERY:
        spr->centerY = matte_value_as_number(heap, args[2]);
        break;

      case SESNSA_LAYER:
        spr->layer = matte_value_as_number(heap, args[2]);
        break;

      case SESNSA_TILEINDEX:
        spr->tile = matte_value_as_number(heap, args[2]);
        break;
      
      case SESNSA_EFFECT:
        spr->effect = matte_value_as_number(heap, args[2]);
        break;

      case SESNSA_PALETTE:
        spr->palette = matte_value_as_number(heap, args[2]);
        break;

            
    }
    return matte_heap_new_value(heap);
}



typedef enum {
    SESNEA_UPDATERATE,
    SESNEA_UPDATEFUNC,
    SESNEA_RESOLUTION,
    SESNEA_ADDALARM,
    SESNEA_REMOVEALARM    
} SESNative_EngineAttribs_t;



matteValue_t ses_native__engine_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    printf("ENGINE   ID: %d\n",
        (int)matte_value_as_number(heap, args[0])
    );  
    
    switch((int)matte_value_as_number(heap, args[0])) {

      // function to call to update each frame, according to the user.
      case SESNEA_UPDATEFUNC:
        matte_value_object_pop_lock(heap, sdl.updateFunc);
        sdl.updateFunc = args[1];
        matte_value_object_push_lock(heap, sdl.updateFunc);
        break;
        
      case SESNEA_UPDATERATE:
        sdl.frameUpdateDelayMS = matte_value_as_number(heap, args[1]) * 1000;
        break;
        
    }
    
    return matte_heap_new_value(heap);
}



typedef enum {
    SESNPA_BACK,
    SESNPA_MIDBACK,
    SESNPA_MIDFRONT,
    SESNPA_FRONT
} SESNative_PaletteAttribs_t;

matteValue_t ses_native__palette_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);

    uint32_t id = matte_value_as_number(heap, args[0]);
    
    if (id >= matte_array_get_size(sdl.palettes)) {
        SES_Palette p = {};
        matte_array_push(sdl.palettes, p);
    }
    SES_Palette * p = &matte_array_at(sdl.palettes, SES_Palette, id);



    switch((int)matte_value_as_number(heap, args[1])) {
      case SESNPA_BACK:
        p->back.x = matte_value_as_number(heap, args[2]);
        p->back.y = matte_value_as_number(heap, args[3]);
        p->back.z = matte_value_as_number(heap, args[4]);
        break;

      case SESNPA_MIDBACK:
        p->midBack.x = matte_value_as_number(heap, args[2]);
        p->midBack.y = matte_value_as_number(heap, args[3]);
        p->midBack.z = matte_value_as_number(heap, args[4]);
        break;

      case SESNPA_MIDFRONT:
        p->midFront.x = matte_value_as_number(heap, args[2]);
        p->midFront.y = matte_value_as_number(heap, args[3]);
        p->midFront.z = matte_value_as_number(heap, args[4]);
        break;

      case SESNPA_FRONT:
        p->front.x = matte_value_as_number(heap, args[2]);
        p->front.y = matte_value_as_number(heap, args[3]);
        p->front.z = matte_value_as_number(heap, args[4]);
        break;

            
    }
    return matte_heap_new_value(heap);
}

typedef enum {
    BIND,
    SETTEXEL,
    UNBIND,
} SESNative_TileAttribs_t;


matteValue_t ses_native__tile_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    printf("TILE     ID: %d, ATTRIB: %d\n",
        (int)matte_value_as_number(heap, args[0]),
        (int)matte_value_as_number(heap, args[1])
    );  
    switch((int)matte_value_as_number(heap, args[1])) {
      case BIND:
        ses_sdl_gl_bind_tile(matte_value_as_number(heap, args[0]));
        break;
        
      case SETTEXEL:
        ses_sdl_gl_set_tile_pixel(
            matte_value_as_number(heap, args[0]),
            matte_value_as_number(heap, args[2])
        );
        break;
        
      case UNBIND:
        ses_sdl_gl_unbind_tile();
        break;
    }
    return matte_heap_new_value(heap);
}
matteValue_t ses_native__input_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    printf("INPUT    ID: %d, ATTRIB: %d\n",
        (int)matte_value_as_number(heap, args[0]),
        (int)matte_value_as_number(heap, args[1])
    );  

    return matte_heap_new_value(heap);
}
matteValue_t ses_native__audio_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    printf("AUDIO    ID: %d, ATTRIB: %d\n",
        (int)matte_value_as_number(heap, args[0]),
        (int)matte_value_as_number(heap, args[1])
    );  
    return matte_heap_new_value(heap);
}

extern matteValue_t ses_native__bg_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    printf("BG       ID: %d, ATTRIB: %d\n",
        (int)matte_value_as_number(heap, args[0]),
        (int)matte_value_as_number(heap, args[1])
    );  
    return matte_heap_new_value(heap);

}




matteValue_t ses_native__palette_query(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    printf("P.QUERY  ID: %d, ATTRIB: %d\n",
        (int)matte_value_as_number(heap, args[0]),
        (int)matte_value_as_number(heap, args[1])
    );  
    return matte_heap_new_value(heap);
}
matteValue_t ses_native__tile_query(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    matteValue_t out = matte_heap_new_value(heap);
    matte_value_into_number(
        heap,
        &out,
        ses_sdl_gl_get_tile_pixel(
            matte_value_as_number(heap, args[0])
        )
    );
    return out;
}

matteValue_t ses_native__bg_query(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    printf("B.QUERY  ID: %d, ATTRIB: %d\n",
        (int)matte_value_as_number(heap, args[0]),
        (int)matte_value_as_number(heap, args[1])
    );  
    return matte_heap_new_value(heap);
}




void ses_native__commit_rom() {
    // nothing yet!
    if (SDL_Init(
        SDL_INIT_TIMER |
        SDL_INIT_VIDEO |
        SDL_INIT_AUDIO |
        SDL_INIT_GAMECONTROLLER            
    ) != 0) {
        printf("SDL2 subsystem init failure.\n");
        exit(1);
    }
    sdl.sprites = matte_array_create(sizeof(SES_Sprite));
    sdl.palettes = matte_array_create(sizeof(SES_Palette));

    ses_sdl_gl_init(
        &sdl.window,
        &sdl.ctx
    );

    sdl.frameUpdateDelayMS = (1 / 60.0)*1000;
    
    SDL_AddTimer(sdl.frameUpdateDelayMS, ses_sdl_emit_frame_event, NULL);
   
}





int ses_native__main_loop(matte_t * m) {
    sdl.vm = matte_get_vm(m);

    SDL_Event evt;
    int quit = 0;
    while(!quit) {
        while(SDL_PollEvent(&evt) != 0) {
            if (evt.type == SDL_QUIT) {
                quit = 1;
                break;
            }            
            
            
            // frame update controlled by timer
            if (evt.type == SDL_USEREVENT) {
                if (sdl.updateFunc.binID) {
                    matte_vm_call(
                        sdl.vm,
                        sdl.updateFunc,
                        matte_array_empty(),
                        matte_array_empty(),
                        NULL
                    );
                }

                ses_sdl_render();
                SDL_GL_SwapWindow(sdl.window);
            }
        }
        
    }
    return 0;
}


