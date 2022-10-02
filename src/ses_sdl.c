#include "matte/src/matte_vm.h"
#include "matte/src/matte.h"
#include "matte/src/matte_array.h"
#include "matte/src/matte_string.h"
#include "linear.h"
#include <stdio.h>

#include <SDL2/SDL.h>

typedef enum {
    SES_DEVICE__KEYBOARD,
    
    SES_DEVICE__POINTER0,
    SES_DEVICE__POINTER1,
    SES_DEVICE__POINTER2,
    SES_DEVICE__POINTER3,

    SES_DEVICE__GAMEPAD0,
    SES_DEVICE__GAMEPAD1,
    SES_DEVICE__GAMEPAD2,
    SES_DEVICE__GAMEPAD3,

    
} SES_DeviceType;


typedef struct {
    // SES_Sprite *
    matteArray_t * sprites;
    matteArray_t * bgs;

} SES_GraphicsLayer;


typedef struct {
    // matteValue_t of functions to call
    matteArray_t * callbacks;

    // uint32_t IDs waiting to be used.
    matteArray_t * dead;
} SES_InputCallbackSet;


typedef struct {

    // array of user callbacks for input.
    SES_InputCallbackSet inputs[SES_DEVICE__GAMEPAD3+1];

    // an array of SES_Sprite, representing all accessed 
    // sprites.
    matteArray_t * sprites;

    // an array of SES_Background, representing all accessed 
    // backgrounds.
    matteArray_t * bgs;
    
    
    // all layers, drawn in order from 0 to 31.
    SES_GraphicsLayer layers[32];
    
    
    // an array of SES_Palette, representing all accessed 
    // palettes
    matteArray_t * palettes;
    
    // user function called every frame    
    matteValue_t updateFunc;


} SES_Context;

typedef struct {
    // SDL window
    SDL_Window    * window;
    
    // SDL GLES context
    SDL_GLContext * ctx;

    
    matteVM_t * vm;

    
    SES_Context main;
    
    SES_Context aux;
    
    
    // delay before updating the next frame
    uint32_t frameUpdateDelayMS;
    
    // callback ID for the timer event emitter
    SDL_TimerID frameUpdateID;
    
    

} SES_SDL;

static SES_SDL sdl = {};



static matteValue_t ses_sdl_sprite_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
static matteValue_t ses_sdl_engine_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
static matteValue_t ses_sdl_palette_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
static matteValue_t ses_sdl_tile_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
static matteValue_t ses_sdl_input_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
static matteValue_t ses_sdl_audio_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
static matteValue_t ses_sdl_bg_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);

static matteValue_t ses_sdl_palette_query(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
static matteValue_t ses_sdl_tile_query(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);




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
    float x;
    float y;
    int layer;
    int effect;
    int enabled;
    int id;

    uint32_t palette;
} SES_Background;




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

// Copies tile data from another tile into the bound tile.
extern void ses_sdl_gl_copy_from(uint32_t otherTileID);

// Unbinds a tile, committing a tiles new data if edited.
extern void ses_sdl_gl_unbind_tile();

extern void ses_sdl_gl_render_begin();

extern void ses_sdl_gl_render_sprite(
    float x, float y,
    float scaleX, float scaleY,
    float centerX, float centerY,
    float rotation,
    int effect,

    sesVector_t back,
    sesVector_t midBack,
    sesVector_t midFront,
    sesVector_t front,
            
    uint32_t id
);

extern void ses_sdl_gl_render_background(
    float x, float y,

    int effect,

    sesVector_t back,
    sesVector_t midBack,
    sesVector_t midFront,
    sesVector_t front,
            
    uint32_t id
);




extern int ses_sdl_gl_get_render_width();

extern int ses_sdl_gl_get_render_height();
 

extern void ses_sdl_gl_render_end();



////////



static void ses_sdl_render() {
    // re-sort sprites and bgs into layer buckets;
    ses_sdl_gl_render_begin();
    uint32_t i, n;
    uint32_t len = matte_array_get_size(sdl.main.sprites);
    SES_Sprite * iter = matte_array_get_data(sdl.main.sprites);
    for(i = 0; i < len; ++i, iter++) {
        if (iter->enabled == 0) continue;

        SES_GraphicsLayer * layer = sdl.main.layers+iter->layer;
        matte_array_push(layer->sprites, iter);
    }



    SES_Background * bg = matte_array_get_data(sdl.main.bgs);
    len = matte_array_get_size(sdl.main.bgs);
    for(i = 0; i < len; ++i, bg++) {
        if (bg->enabled == 0) continue;


        SES_GraphicsLayer * layer = sdl.main.layers+bg->layer;
        matte_array_push(layer->bgs, bg);
    }




    // draw each layer in order
    for(i = 0; i < 32; ++i) {
        SES_GraphicsLayer * layer = sdl.main.layers+i;
    
        // start with backgrounds
        len = matte_array_get_size(layer->bgs);
        if (len) { 
            for(n = 0; n < len; ++n) {        
                bg = matte_array_at(layer->bgs, SES_Background *, n);
            
                SES_Palette p = {};
                if (bg->palette < matte_array_get_size(sdl.main.palettes)) {
                    p = matte_array_at(sdl.main.palettes, SES_Palette, bg->palette);
                }
                
                ses_sdl_gl_render_background(
                    bg->x, bg->y,
                    bg->effect,
                    
                    p.back,
                    p.midBack,
                    p.midFront,
                    p.front,
                    
                    bg->id
                );

            }
            matte_array_set_size(layer->bgs, 0);

        }
        // then do sprites
        len = matte_array_get_size(layer->sprites);
        if (!len) continue;
        for(n = 0; n < len; ++n) {        
            iter = matte_array_at(layer->sprites, SES_Sprite *, n);
        
            SES_Palette p = {};
            if (iter->palette < matte_array_get_size(sdl.main.palettes)) {
                p = matte_array_at(sdl.main.palettes, SES_Palette, iter->palette);
            }
            
            ses_sdl_gl_render_sprite(
                iter->x, iter->y,
                iter->scaleX, iter->scaleY,
                iter->centerX, iter->centerY,
                iter->rotation,
                iter->effect,
                
                p.back,
                p.midBack,
                p.midFront,
                p.front,
                
                iter->tile
            );
        }
        
        matte_array_set_size(layer->sprites, 0);
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




matteValue_t ses_sdl_sprite_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);

    uint32_t id = matte_value_as_number(heap, args[0]);
    while(id >= matte_array_get_size(sdl.main.sprites)) {
        SES_Sprite spr = {};
        matte_array_push(sdl.main.sprites, spr);
    }
    SES_Sprite * spr = &matte_array_at(sdl.main.sprites, SES_Sprite, id);
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



matteValue_t ses_sdl_engine_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    printf("ENGINE   ID: %d\n",
        (int)matte_value_as_number(heap, args[0])
    );  
    
    switch((int)matte_value_as_number(heap, args[0])) {

      // function to call to update each frame, according to the user.
      case SESNEA_UPDATEFUNC:
        matte_value_object_pop_lock(heap, sdl.main.updateFunc);
        sdl.main.updateFunc = args[1];
        matte_value_object_push_lock(heap, sdl.main.updateFunc);
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

matteValue_t ses_sdl_palette_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);

    uint32_t id = matte_value_as_number(heap, args[0]);
    
    if (id >= matte_array_get_size(sdl.main.palettes)) {
        SES_Palette p = {};
        matte_array_push(sdl.main.palettes, p);
    }
    SES_Palette * p = &matte_array_at(sdl.main.palettes, SES_Palette, id);



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
    SESNTA_BIND,
    SESNTA_SETTEXEL,
    SESNTA_UNBIND,
    SESNTA_COPY
} SESNative_TileAttribs_t;


matteValue_t ses_sdl_tile_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    printf("TILE     ID: %d, ATTRIB: %d\n",
        (int)matte_value_as_number(heap, args[0]),
        (int)matte_value_as_number(heap, args[1])
    );  
    switch((int)matte_value_as_number(heap, args[1])) {
      case SESNTA_BIND:
        ses_sdl_gl_bind_tile(matte_value_as_number(heap, args[0]));
        break;
        
      case SESNTA_SETTEXEL:
        ses_sdl_gl_set_tile_pixel(
            matte_value_as_number(heap, args[0]),
            matte_value_as_number(heap, args[2])
        );
        break;
        
      case SESNTA_UNBIND:
        ses_sdl_gl_unbind_tile();
        break;
        
      case SESNTA_COPY:
        ses_sdl_gl_copy_from(
            matte_value_as_number(heap, args[0])
        );
        break;


    }
    return matte_heap_new_value(heap);
}


typedef enum {
    SESNIA_ADD,
    SESNIA_REMOVE
} SESNative_InputAction_t;

matteValue_t ses_sdl_input_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    printf("INPUT    ID: %d, ATTRIB: %d\n",
        (int)matte_value_as_number(heap, args[0]),
        (int)matte_value_as_number(heap, args[1])
    );  

    switch((int)matte_value_as_number(heap, args[0])) {
      case SESNIA_ADD: {
        SES_InputCallbackSet * set = &sdl.main.inputs[(int)matte_value_as_number(heap, args[1])];

        matteValue_t out = matte_heap_new_value(heap);
        uint32_t id;
        if (matte_array_get_size(set->dead)) {
            id = matte_array_at(set->dead, uint32_t, matte_array_get_size(set->dead)-1);
            matte_array_set_size(set->dead, matte_array_get_size(set->dead)-1);
        } else {
            matte_array_push(set->callbacks, args[2]);
            id = matte_array_get_size(set->callbacks)-1;
        }
        matte_array_at(set->callbacks, matteValue_t, id) = args[2];
        matte_value_object_push_lock(heap, args[2]);
        matte_value_into_number(heap, &out, id);
        return out;
      }
      
      
      case SESNIA_REMOVE: {
        SES_InputCallbackSet * set = &sdl.main.inputs[(int)matte_value_as_number(heap, args[1])];
        uint32_t id = matte_value_as_number(heap, args[2]);      
        
        if (id > matte_array_get_size(set->callbacks) ||
            matte_array_at(set->callbacks, matteValue_t, id).binID == 0) {
            break;   
        }
        matte_array_at(set->callbacks, matteValue_t, id).binID = 0;
        matte_array_push(set->dead, id);
      }
            
    }



    return matte_heap_new_value(heap);
}
matteValue_t ses_sdl_audio_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    printf("AUDIO    ID: %d, ATTRIB: %d\n",
        (int)matte_value_as_number(heap, args[0]),
        (int)matte_value_as_number(heap, args[1])
    );  
    return matte_heap_new_value(heap);
}



typedef enum {
    SESNBA_ENABLE,
    SESNBA_POSITIONX,
    SESNBA_POSITIONY,
    SESNBA_LAYER,
    SESNBA_EFFECT,
    SESNBA_PALETTE

} SESNative_BackgroundAttribs_t;

matteValue_t ses_sdl_bg_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    /*
    printf("BG       ID: %d, ATTRIB: %d\n",
        (int)matte_value_as_number(heap, args[0]),
        (int)matte_value_as_number(heap, args[1])
    );
    */  

    uint32_t id = matte_value_as_number(heap, args[0]);
    while(id >= matte_array_get_size(sdl.main.bgs)) {
        SES_Background bg = {};
        bg.id = id;
        matte_array_push(sdl.main.bgs, bg);
    }
    SES_Background * bg = &matte_array_at(sdl.main.bgs, SES_Background, id);
    switch((int)matte_value_as_number(heap, args[1])) {
      case SESNBA_ENABLE:
        bg->enabled = matte_value_as_number(heap, args[2]);
        break;

      case SESNBA_POSITIONX:
        bg->x = matte_value_as_number(heap, args[2]);
        break;
      
      case SESNBA_POSITIONY:
        bg->y = matte_value_as_number(heap, args[2]);
        break;

      case SESNBA_LAYER:
        bg->layer = matte_value_as_number(heap, args[2]);
        break;
      
      case SESNBA_EFFECT:
        bg->effect = matte_value_as_number(heap, args[2]);
        break;

      case SESNBA_PALETTE:
        bg->palette = matte_value_as_number(heap, args[2]);
        break;

            
    }
    return matte_heap_new_value(heap);
}




matteValue_t ses_sdl_palette_query(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    printf("P.QUERY  ID: %d, ATTRIB: %d\n",
        (int)matte_value_as_number(heap, args[0]),
        (int)matte_value_as_number(heap, args[1])
    );  
    return matte_heap_new_value(heap);
}
matteValue_t ses_sdl_tile_query(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
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




static SES_Context context_create() {
    SES_Context ctx = {};

    ctx.sprites = matte_array_create(sizeof(SES_Sprite));
    ctx.bgs = matte_array_create(sizeof(SES_Background));
    ctx.palettes = matte_array_create(sizeof(SES_Palette));
    int i;
    for(i = 0; i <= SES_DEVICE__GAMEPAD3; ++i) {
        ctx.inputs[i].callbacks = matte_array_create(sizeof(matteValue_t));    
        ctx.inputs[i].dead = matte_array_create(sizeof(uint32_t));
    }
    
    for(i = 0; i < 32; ++i) {
        SES_GraphicsLayer * layer = &ctx.layers[i];
        layer->sprites = matte_array_create(sizeof(SES_Sprite *));
        layer->bgs = matte_array_create(sizeof(SES_Background *));
    }
    return ctx;
}

void ses_native_commit_rom(matte_t * m) {
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
    matteVM_t * vm = matte_get_vm(m);

    // all 3 modes require activating the core features.
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__sprite_attrib"), 3, ses_sdl_sprite_attrib, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__engine_attrib"), 3, ses_sdl_engine_attrib, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__palette_attrib"), 5, ses_sdl_palette_attrib, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__tile_attrib"), 3, ses_sdl_tile_attrib, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__input_attrib"), 3, ses_sdl_input_attrib, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__audio_attrib"), 4, ses_sdl_audio_attrib, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__bg_attrib"), 4, ses_sdl_bg_attrib, NULL);


    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__palette_query"), 3, ses_sdl_palette_query, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__tile_query"), 2, ses_sdl_tile_query, NULL);


    sdl.main = context_create();
    sdl.aux = context_create();    
    ses_sdl_gl_init(
        &sdl.window,
        &sdl.ctx
    );

    sdl.frameUpdateDelayMS = (1 / 60.0)*1000;
    
    SDL_AddTimer(sdl.frameUpdateDelayMS, ses_sdl_emit_frame_event, NULL);
   
}

void ses_native_swap_context() {
    SES_Context c = sdl.main;
    sdl.main = sdl.aux;
    sdl.aux = c;
}

int ses_native_update(matte_t * m) {
    sdl.vm = matte_get_vm(m);
    matteHeap_t * heap = matte_vm_get_heap(sdl.vm);
    SDL_Event evt;

    
    while(SDL_PollEvent(&evt) != 0) {
        if (evt.type == SDL_QUIT) {
            return 0;
            break;
        }            
        
        
        
        switch(evt.type) {
          case SDL_KEYUP: 
          case SDL_KEYDOWN: {          
          
            uint32_t i;
            uint32_t len = matte_array_get_size(sdl.main.inputs[SES_DEVICE__KEYBOARD].callbacks);
            if (len == 0) break;
            
            matteString_t * textStr = (matteString_t*)MATTE_VM_STR_CAST(sdl.vm, "key");
            matteString_t * eventStr = (matteString_t*)MATTE_VM_STR_CAST(sdl.vm, "event");
            
            
            matteValue_t text = matte_heap_new_value(heap);
            matteValue_t event = matte_heap_new_value(heap);
            
            matte_value_into_string(heap, &text, textStr);
            matte_value_into_string(heap, &event, eventStr);
            
            
            matteValue_t textval = matte_heap_new_value(heap);
            matteValue_t eventVal = matte_heap_new_value(heap);
            
            double xcon, ycon;
            int w, h;
            
            
            matte_value_into_number(heap, &textval, evt.key.keysym.sym);
            matte_value_into_number(heap, &eventVal, (evt.type == SDL_KEYDOWN ? 2 : 6));// key down



            matteValue_t namesArr[] = {event, text};
            matteValue_t valsArr[] = {eventVal, textval};                
            
            for(i = 0; i < len; ++i) {
                matteValue_t val = matte_array_at(sdl.main.inputs[SES_DEVICE__KEYBOARD].callbacks, matteValue_t, i);    
                if (val.binID == 0) continue;

                // for safety
                matteArray_t names = MATTE_ARRAY_CAST(namesArr, matteValue_t, 2);
                matteArray_t vals = MATTE_ARRAY_CAST(valsArr, matteValue_t, 2);

                matte_vm_call(sdl.vm, val, &vals, &names, NULL);
                
            }   
            break;                 
          } 
        
          case SDL_TEXTINPUT: {
          
          
            uint32_t i;
            uint32_t len = matte_array_get_size(sdl.main.inputs[SES_DEVICE__KEYBOARD].callbacks);
            if (len == 0) break;
            
            matteString_t * textStr = (matteString_t*)MATTE_VM_STR_CAST(sdl.vm, "text");
            matteString_t * eventStr = (matteString_t*)MATTE_VM_STR_CAST(sdl.vm, "event");
            
            
            matteValue_t text = matte_heap_new_value(heap);
            matteValue_t event = matte_heap_new_value(heap);
            
            matte_value_into_string(heap, &text, textStr);
            matte_value_into_string(heap, &event, eventStr);
            
            
            matteValue_t textval = matte_heap_new_value(heap);
            matteValue_t eventVal = matte_heap_new_value(heap);
            
            double xcon, ycon;
            int w, h;
            
            
            matteString_t * textRaw = matte_string_create_from_c_str("%s", evt.text.text);
            matte_value_into_string(heap, &textval, textRaw);
            matte_string_destroy(textRaw);
            matte_value_into_number(heap, &eventVal, 1);



            matteValue_t namesArr[] = {event, text};
            matteValue_t valsArr[] = {eventVal, textval};                
            
            for(i = 0; i < len; ++i) {
                matteValue_t val = matte_array_at(sdl.main.inputs[SES_DEVICE__KEYBOARD].callbacks, matteValue_t, i);    
                if (val.binID == 0) continue;

                // for safety
                matteArray_t names = MATTE_ARRAY_CAST(namesArr, matteValue_t, 2);
                matteArray_t vals = MATTE_ARRAY_CAST(valsArr, matteValue_t, 2);

                matte_vm_call(sdl.vm, val, &vals, &names, NULL);
                
            }   
            break;       
          }
        
        
          case SDL_MOUSEBUTTONDOWN:
          case SDL_MOUSEBUTTONUP: {
          
          
            uint32_t i;
            uint32_t len = matte_array_get_size(sdl.main.inputs[SES_DEVICE__POINTER0].callbacks);
            if (len == 0) break;
            
            matteString_t * xStr = (matteString_t*)MATTE_VM_STR_CAST(sdl.vm, "x");
            matteString_t * yStr = (matteString_t*)MATTE_VM_STR_CAST(sdl.vm, "y");
            matteString_t * buttonStr = (matteString_t*)MATTE_VM_STR_CAST(sdl.vm, "button");
            matteString_t * eventStr = (matteString_t*)MATTE_VM_STR_CAST(sdl.vm, "event");
            
            
            matteValue_t x = matte_heap_new_value(heap);
            matteValue_t y = matte_heap_new_value(heap);
            matteValue_t event = matte_heap_new_value(heap);
            matteValue_t button = matte_heap_new_value(heap);
            
            matte_value_into_string(heap, &x, xStr);
            matte_value_into_string(heap, &y, yStr);
            matte_value_into_string(heap, &button, buttonStr);
            matte_value_into_string(heap, &event, eventStr);
            
            
            matteValue_t xval = matte_heap_new_value(heap);
            matteValue_t yval = matte_heap_new_value(heap);
            matteValue_t buttonval = matte_heap_new_value(heap);
            matteValue_t eventVal = matte_heap_new_value(heap);
            
            double xcon, ycon;
            int w, h;
            SDL_GetWindowSize(sdl.window, &w, &h);
            
            
            
            matte_value_into_number(heap, &xval, (evt.button.x / (float) w) * ses_sdl_gl_get_render_width());
            matte_value_into_number(heap, &yval, (evt.button.y / (float) h) * ses_sdl_gl_get_render_height());
            matte_value_into_number(heap, &eventVal, evt.button.type == SDL_MOUSEBUTTONDOWN ? 3 : 4);

            int which;
            switch(evt.button.button) {
              case SDL_BUTTON_LEFT:   which = 0; break;
              case SDL_BUTTON_MIDDLE: which = 1; break;
              case SDL_BUTTON_RIGHT:  which = 2; break;
            }

            matte_value_into_number(heap, &buttonval, evt.button.button);



            matteValue_t namesArr[] = {event, x, y, button};
            matteValue_t valsArr[] = {eventVal, xval, yval, buttonval};                
            
            for(i = 0; i < len; ++i) {
                matteValue_t val = matte_array_at(sdl.main.inputs[SES_DEVICE__POINTER0].callbacks, matteValue_t, i);    
                if (val.binID == 0) continue;

                // for safety
                matteArray_t names = MATTE_ARRAY_CAST(namesArr, matteValue_t, 4);
                matteArray_t vals = MATTE_ARRAY_CAST(valsArr, matteValue_t, 4);

                matte_vm_call(sdl.vm, val, &vals, &names, NULL);
                
            }
            break;
          }


          case SDL_MOUSEWHEEL: {
          
          
            uint32_t i;
            uint32_t len = matte_array_get_size(sdl.main.inputs[SES_DEVICE__POINTER0].callbacks);
            if (len == 0) break;
            
            matteString_t * xStr = (matteString_t*)MATTE_VM_STR_CAST(sdl.vm, "x");
            matteString_t * yStr = (matteString_t*)MATTE_VM_STR_CAST(sdl.vm, "y");
            matteString_t * eventStr = (matteString_t*)MATTE_VM_STR_CAST(sdl.vm, "event");
            
            
            matteValue_t x = matte_heap_new_value(heap);
            matteValue_t y = matte_heap_new_value(heap);
            matteValue_t event = matte_heap_new_value(heap);
            
            matte_value_into_string(heap, &x, xStr);
            matte_value_into_string(heap, &y, yStr);
            matte_value_into_string(heap, &event, eventStr);
            
            
            matteValue_t xval = matte_heap_new_value(heap);
            matteValue_t yval = matte_heap_new_value(heap);
            matteValue_t eventVal = matte_heap_new_value(heap);
            
            
            
            
            matte_value_into_number(heap, &xval, evt.wheel.x);
            matte_value_into_number(heap, &yval, evt.wheel.y);
            matte_value_into_number(heap, &eventVal, 5); // scroll



            matteValue_t namesArr[] = {event, x, y};
            matteValue_t valsArr[] = {eventVal, xval, yval};                
            
            for(i = 0; i < len; ++i) {
                matteValue_t val = matte_array_at(sdl.main.inputs[SES_DEVICE__POINTER0].callbacks, matteValue_t, i);    
                if (val.binID == 0) continue;

                // for safety
                matteArray_t names = MATTE_ARRAY_CAST(namesArr, matteValue_t, 3);
                matteArray_t vals = MATTE_ARRAY_CAST(valsArr, matteValue_t, 3);

                matte_vm_call(sdl.vm, val, &vals, &names, NULL);
                
            }
            break;
          }

        
          case SDL_MOUSEMOTION: {
          
          
            uint32_t i;
            uint32_t len = matte_array_get_size(sdl.main.inputs[SES_DEVICE__POINTER0].callbacks);
            if (len == 0) break;
            
            matteString_t * xStr = (matteString_t*)MATTE_VM_STR_CAST(sdl.vm, "x");
            matteString_t * yStr = (matteString_t*)MATTE_VM_STR_CAST(sdl.vm, "y");
            matteString_t * eventStr = (matteString_t*)MATTE_VM_STR_CAST(sdl.vm, "event");
            
            
            matteValue_t x = matte_heap_new_value(heap);
            matteValue_t y = matte_heap_new_value(heap);
            matteValue_t event = matte_heap_new_value(heap);
            
            matte_value_into_string(heap, &x, xStr);
            matte_value_into_string(heap, &y, yStr);
            matte_value_into_string(heap, &event, eventStr);
            
            
            matteValue_t xval = matte_heap_new_value(heap);
            matteValue_t yval = matte_heap_new_value(heap);
            matteValue_t eventVal = matte_heap_new_value(heap);
            
            double xcon, ycon;
            int w, h;
            SDL_GetWindowSize(sdl.window, &w, &h);
            
            
            
            matte_value_into_number(heap, &xval, (evt.motion.x / (float) w) * ses_sdl_gl_get_render_width());
            matte_value_into_number(heap, &yval, (evt.motion.y / (float) h) * ses_sdl_gl_get_render_height());
            matte_value_into_number(heap, &eventVal, 0);



            matteValue_t namesArr[] = {event, x, y};
            matteValue_t valsArr[] = {eventVal, xval, yval};                
            
            for(i = 0; i < len; ++i) {
                matteValue_t val = matte_array_at(sdl.main.inputs[SES_DEVICE__POINTER0].callbacks, matteValue_t, i);    
                if (val.binID == 0) continue;

                // for safety
                matteArray_t names = MATTE_ARRAY_CAST(namesArr, matteValue_t, 3);
                matteArray_t vals = MATTE_ARRAY_CAST(valsArr, matteValue_t, 3);

                matte_vm_call(sdl.vm, val, &vals, &names, NULL);
                
            }
          }
            
        }
        
        
        // frame update controlled by timer
        if (evt.type == SDL_USEREVENT) {
            if (sdl.main.updateFunc.binID) {
                matte_vm_call(
                    sdl.vm,
                    sdl.main.updateFunc,
                    matte_array_empty(),
                    matte_array_empty(),
                    NULL
                );
            }

            ses_sdl_render();
            SDL_GL_SwapWindow(sdl.window);
        }
    }
    SDL_Delay(1);
    
    return 1;
}


int ses_native_main_loop(matte_t * m) {
    while(ses_native_update(m)) {}
    return 0;
}


