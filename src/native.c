#include "matte/src/matte_vm.h"
#include "matte/src/matte.h"
#include "matte/src/matte_array.h"
#include "matte/src/matte_string.h"
#include <stdio.h>
#include "native.h"
#include "window.h"
#include "cartridge.h"
#include <math.h>
#define LAYER_MIN -63
#define LAYER_MID 0
#define LAYER_MAX 64
#define SPRITE_MAX 4096
#define OSCILLATOR_MAX 1024
#define SPRITE_COUNT_TOTAL 65536
#define TILE_ID_MAX 0x8ffff


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
    // matteValue_t of functions to call
    matteArray_t * callbacks;

    // uint32_t IDs waiting to be used.
    matteArray_t * dead;
} SES_InputCallbackSet;







typedef struct {
    SES_InputCallbackSet inputs[SES_DEVICE__GAMEPAD3+1];

    // user function called every frame    
    matteValue_t updateFunc;


} sesContext_t;





typedef struct {
    // native window
    sesWindow_t * window;

    sesGraphicsContext_t * graphics;
    
    matteVM_t * vm;



    sesContext_t mainContext;

    // every cartridge has a 
    sesCartridge_t * mainCart;
    
    // whether the main and aux are swapped. Usually for debugging context
    int swapped;


    // current pointer
    int hasPointerMotionEvent;
    int hasPointerScrollEvent;


    int pointerX;
    int pointerY;
    int pointerScrollX;
    int pointerScrollY;    

} sesNative_t;

static sesNative_t ses = {};



static matteValue_t ses_native_get_calling_bank(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);


static matteValue_t ses_native_sprite_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
static matteValue_t ses_native_engine_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
static matteValue_t ses_native_palette_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
static matteValue_t ses_native_tile_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
static matteValue_t ses_native_input_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
static matteValue_t ses_native_audio_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
static matteValue_t ses_native_bg_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);

static matteValue_t ses_native_palette_query(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
static matteValue_t ses_native_tile_query(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);








static void ses_native_render() {
    // re-sort sprites and bgs into layer buckets;
    ses_cartridge_push_graphics(ses.mainCart, ses.graphics);
    ses_graphics_context_render(ses.graphics);
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
    sesGraphicsContext_Sprite_t * spr = ses_cartridge_get_sprite(ses.mainCart, id);
    
    if (!spr) {
        matte_vm_raise_error_string(vm, MATTE_VM_STR_CAST(vm, "Sprite accessed beyond limit"));                
        return matte_heap_new_value(heap);    
    }


    uint32_t len = matte_value_object_get_number_key_count(heap, args[1]);
    uint32_t i;
    for(i = 0; i < len; i+=2) {
        matteValue_t * flag  = matte_value_object_array_at_unsafe(heap, args[1], i);
        matteValue_t * value = matte_value_object_array_at_unsafe(heap, args[1], i+1);

        switch((int)matte_value_as_number(heap, *flag)) {
          case SESNSA_ENABLE: {
            ses_cartridge_enable_sprite(ses.mainCart, id, matte_value_as_number(heap, *value));
            break;
          }  
          case SESNSA_ROTATION:
            spr->rotation = matte_value_as_number(heap, *value);
            break;

          case SESNSA_SCALEX:
            spr->scaleX = matte_value_as_number(heap, *value);
            break;

          case SESNSA_SCALEY:
            spr->scaleY = matte_value_as_number(heap, *value);
            break;

          case SESNSA_POSITIONX:
            spr->x = matte_value_as_number(heap, *value);
            break;
          
          case SESNSA_POSITIONY:
            spr->y = matte_value_as_number(heap, *value);
            break;

          case SESNSA_CENTERX:
            spr->centerX = matte_value_as_number(heap, *value);
            break;

          case SESNSA_CENTERY:
            spr->centerY = matte_value_as_number(heap, *value);
            break;

          case SESNSA_LAYER:
            spr->layer = matte_value_as_number(heap, *value);
            if (spr->layer > LAYER_MAX) spr->layer = LAYER_MAX;
            if (spr->layer < LAYER_MIN) spr->layer = LAYER_MIN;
            break;

          case SESNSA_TILEINDEX:
            spr->tile = matte_value_as_number(heap, *value);
            break;
          
          case SESNSA_EFFECT:
            spr->effect = matte_value_as_number(heap, *value);
            break;

          case SESNSA_PALETTE:
            spr->palette = matte_value_as_number(heap, *value);
            break;
        }

    }
    return matte_heap_new_value(heap);
}





typedef enum {
    SESNOA_ENABLE,
    SESNOA_PERIODMS,
    SESNOA_ONCYCLE,
    SESNOA_GET
} SESNative_OscillatorAttribs;



matteValue_t ses_sdl_oscillator_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);

    uint32_t id = matte_value_as_number(heap, args[0]);
    sesCartridge_Oscillator_t * osc = ses_cartridge_get_oscillator(ses.mainCart, id);
    
    if (!osc) {
        matte_vm_raise_error_string(vm, MATTE_VM_STR_CAST(vm, "Oscillator accessed beyond limit"));
        return matte_heap_new_value(heap);
    }
    

    uint32_t len = matte_value_object_get_number_key_count(heap, args[1]);
    uint32_t i;
    int n;
    for(i = 0; i < len; i+=2) {
        matteValue_t * flag  = matte_value_object_array_at_unsafe(heap, args[1], i);
        matteValue_t * value = matte_value_object_array_at_unsafe(heap, args[1], i+1);

        switch((int)matte_value_as_number(heap, *flag)) {
          case SESNOA_ENABLE:
            ses_cartridge_enable_oscillator(ses.mainCart, id, matte_value_as_boolean(heap, *value));
            break;
            
          case SESNOA_PERIODMS:
            osc->lengthMS = matte_value_as_number(heap, *value);
            osc->endMS = osc->startMS + osc->lengthMS;
            break;
            
          case SESNOA_ONCYCLE:
            if (osc->function.value.id == value->value.id) break;
            if (osc->function.binID) {
                matte_value_object_pop_lock(heap, osc->function);
            }
            osc->function = *value;
            matte_value_object_push_lock(heap, osc->function);
            break;            
            
            
          case SESNOA_GET: {
            uint32_t prog = (osc->endMS - ses_window_get_ticks(ses.window)) / (double)osc->lengthMS;
            if (prog < 0) prog = 0;
            if (prog > 1) prog = 1;
            double frac = 0.5*(1+sin((prog) * (2*M_PI)));
            matteValue_t fracVal = matte_heap_new_value(heap);
            matte_value_into_number(heap, &fracVal, frac);
            return fracVal;
          }
    
        }

    }
    return matte_heap_new_value(heap);
}





typedef enum {
    SESNEA_UPDATERATE,
    SESNEA_UPDATEFUNC,
    SESNEA_RESOLUTION,
    SESNEA_CLIPBOARDGET,
    SESNEA_CLIPBOARDSET 
} SESNative_EngineAttribs_t;



matteValue_t ses_sdl_engine_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    printf("ENGINE   ID: %d\n",
        (int)matte_value_as_number(heap, args[0])
    );  
    
    switch((int)matte_value_as_number(heap, args[0])) {

      // function to call to update each frame, according to the user.
      case SESNEA_UPDATEFUNC:
        matte_value_object_pop_lock(heap, ses.mainContext.updateFunc);
        ses.mainContext.updateFunc = args[1];
        matte_value_object_push_lock(heap, ses.mainContext.updateFunc);
        break;
        
      case SESNEA_UPDATERATE:
        ses_window_set_frame_update_delay(ses.window, matte_value_as_number(heap, args[1]) * 1000);
        break;


      case SESNEA_CLIPBOARDGET: {
        matteValue_t strOut = matte_heap_new_value(heap);
        matteString_t * strVal = ses_window_get_clipboard(ses.window);
        matte_value_into_string(heap, &strOut, strVal);
        matte_string_destroy(strVal);

        return strOut;
      };

      case SESNEA_CLIPBOARDSET: {
        const matteString_t * str = matte_value_string_get_string_unsafe(heap, args[1]);
        ses_window_set_clipboard(ses.window, str);
      };



        
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
    const sesGraphicsContext_Palette_t * pin = ses_graphics_context_storage_get_palette(
        ses_cartridge_get_context_storage(
            ses.mainCart
        ),
        id
    );
    if (!pin) {
        matte_vm_raise_error_string(vm, MATTE_VM_STR_CAST(vm, "Palette accessed beyond limit"));
        return matte_heap_new_value(heap);
    }
    sesGraphicsContext_Palette_t p = *pin;


    switch((int)matte_value_as_number(heap, args[1])) {
      case SESNPA_BACK:
        p.back.x = matte_value_as_number(heap, args[2]);
        p.back.y = matte_value_as_number(heap, args[3]);
        p.back.z = matte_value_as_number(heap, args[4]);
        break;

      case SESNPA_MIDBACK:
        p.midBack.x = matte_value_as_number(heap, args[2]);
        p.midBack.y = matte_value_as_number(heap, args[3]);
        p.midBack.z = matte_value_as_number(heap, args[4]);
        break;

      case SESNPA_MIDFRONT:
        p.midFront.x = matte_value_as_number(heap, args[2]);
        p.midFront.y = matte_value_as_number(heap, args[3]);
        p.midFront.z = matte_value_as_number(heap, args[4]);
        break;

      case SESNPA_FRONT:
        p.front.x = matte_value_as_number(heap, args[2]);
        p.front.y = matte_value_as_number(heap, args[3]);
        p.front.z = matte_value_as_number(heap, args[4]);
        break;

            
    }
    
    
    ses_graphics_context_storage_set_palette(
        ses_cartridge_get_context_storage(
            ses.mainCart
        ),
        id,
        &p
    );
    return matte_heap_new_value(heap);
}

typedef enum {
    SESNTA_SET,
    SESNTA_COPY
} SESNative_TileAttribs_t;


matteValue_t ses_sdl_tile_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    printf("TILE     ID: %d, ATTRIB: %d\n",
        (int)matte_value_as_number(heap, args[0]),
        (int)matte_value_as_number(heap, args[1])
    );  

    uint32_t id = matte_value_as_number(heap, args[0]);
    const sesGraphicsContext_Tile_t * tilep = ses_graphics_context_storage_get_tile(
        ses_cartridge_get_context_storage(
            ses.mainCart
        ),
        id    
    );
    
    if (!tilep) {
        matte_vm_raise_error_string(vm, MATTE_VM_STR_CAST(vm, "Tile accessed beyond limit"));
        return matte_heap_new_value(heap);    
    }

    sesGraphicsContext_Tile_t tile = *tilep;


    switch((int)matte_value_as_number(heap, args[1])) {
        
      case SESNTA_SET: {
        matteValue_t array = args[2];
        int pixelCount = SES_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS*SES_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS;
        if (matte_value_object_get_number_key_count(heap, array) != pixelCount) {
            matte_vm_raise_error_string(vm, MATTE_VM_STR_CAST(vm, "Tile data is not the correct number of members."));
            return matte_heap_new_value(heap);    
        }
        
        int i;
        matteValue_t value;
        for(i = 0; i < pixelCount; ++i) {
            matteValue_t v = matte_value_object_access_index(heap, array, i);
            tile.data[i] = (int)matte_value_as_number(heap, v);
        }
        
        ses_graphics_context_storage_set_tile(
            ses_cartridge_get_context_storage(
                ses.mainCart
            ),
            id,
            &tile            
        );
        
        break;
      }
        
      case SESNTA_COPY: {
        uint32_t tid = matte_value_as_number(heap, args[2]);
        const sesGraphicsContext_Tile_t * tilet = ses_graphics_context_storage_get_tile(
            ses_cartridge_get_context_storage(
                ses.mainCart
            ),
            tid    
        );
        if (!tilet) {
            matte_vm_raise_error_string(vm, MATTE_VM_STR_CAST(vm, "Tile (target of copy) accessed beyond limit"));
            return matte_heap_new_value(heap);    
        }
        
        ses_graphics_context_storage_set_tile(
            ses_cartridge_get_context_storage(
                ses.mainCart
            ),
            tid,
            &tile           
        );
        

        break;
      }

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
        SES_InputCallbackSet * set = &ses.mainContext.inputs[(int)matte_value_as_number(heap, args[1])];

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
        SES_InputCallbackSet * set = &ses.mainContext.inputs[(int)matte_value_as_number(heap, args[1])];
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
    sesGraphicsContext_Background_t * bg = ses_cartridge_get_background(ses.mainCart, id);
    
    if (!bg) {
        matte_vm_raise_error_string(vm, MATTE_VM_STR_CAST(vm, "BG accessed beyond limit"));                
        return matte_heap_new_value(heap);    
    }
    
    
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
        if (bg->layer > LAYER_MAX) bg->layer = LAYER_MAX;
        if (bg->layer < LAYER_MIN) bg->layer = LAYER_MIN;
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
    
    uint32_t id = matte_value_as_number(heap, args[0]);
    const sesGraphicsContext_Palette_t * p = ses_graphics_context_storage_get_palette(
        ses_cartridge_get_context_storage(
            ses.mainCart
        ),
        id
    );
    if (!p) {
        matte_vm_raise_error_string(vm, MATTE_VM_STR_CAST(vm, "Palette accessed beyond limit"));
        return matte_heap_new_value(heap);
    }   

    const sesVector_t * color;
    switch((int)matte_value_as_number(heap, args[1])) {
      case SESNPA_BACK:
        color = &p->back;
        break;

      case SESNPA_MIDBACK:
        color = &p->midBack;
        break;

      case SESNPA_MIDFRONT:
        color = &p->midFront;
        break;

      case SESNPA_FRONT:
        color = &p->front;
        break;

      default:    return matte_heap_new_value(heap);
    }
    
    matteValue_t out = matte_heap_new_value(heap);
    
    switch((int)matte_value_as_number(heap, args[2])) {
      case 0: matte_value_into_number(heap, &out, color->x); break;
      case 1: matte_value_into_number(heap, &out, color->y); break;
      case 2: matte_value_into_number(heap, &out, color->z); break;
    }
    
    return out;
    
    

}
matteValue_t ses_sdl_tile_query(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    matteValue_t out = matte_heap_new_value(heap);

    uint32_t id = matte_value_as_number(heap, args[0]);
    const sesGraphicsContext_Tile_t * tilep = ses_graphics_context_storage_get_tile(
        ses_cartridge_get_context_storage(
            ses.mainCart
        ),
        id    
    );
    
    
    int i;
    int count = SES_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS * SES_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS;
    matteArray_t * arr = matte_array_create(sizeof(matteValue_t));    
    
    for(i = 0; i < count; ++i) {
        matteValue_t m = matte_heap_new_value(heap);
        matte_value_into_number(heap, &m, tilep->data[i]);
        matte_array_push(arr, m);
    }


    matte_value_into_new_object_array_ref(heap, &out, arr);
    matte_array_destroy(arr);
    return out;
}




static sesContext_t context_create() {
    sesContext_t ctx = {};
    /*
    SES_Context ctx = {};

    ctx.bgs = matte_array_create(sizeof(SES_Background));
    ctx.palettes = matte_array_create(sizeof(SES_Palette));
    ctx.sprites = calloc(1, sizeof(SES_Sprite) * SPRITE_COUNT_TOTAL);
    int i;
    for(i = 0; i < SPRITE_COUNT_TOTAL; ++i) {
        ctx.sprites[i].scaleX = 1;
        ctx.sprites[i].scaleY = 1;
    }
    */
    int i;
    for(i = 0; i <= SES_DEVICE__GAMEPAD3; ++i) {
        ctx.inputs[i].callbacks = matte_array_create(sizeof(matteValue_t));    
        ctx.inputs[i].dead = matte_array_create(sizeof(uint32_t));
    }
    

    /*
    for(i = 0; i < 128; ++i) {
        SES_GraphicsLayer * layer = &ctx.layers[i];
        layer->sprites = matte_array_create(sizeof(SES_Sprite *));
        layer->bgs = matte_array_create(sizeof(SES_Background *));
    }
    return ctx;
    */
    return ctx;
}

void ses_native_commit_rom(sesROM_t * rom, matte_t * m) {
    // nothing yet!
    ses.window = ses_window_create();
    ses.graphics = ses_window_get_graphics(ses.window);
    ses.mainCart = ses_cartridge_create(rom, ses.graphics);
    matteVM_t * vm = matte_get_vm(m);

    // all 3 modes require activating the core features.
    //matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__get_calling_bank"), 0, ses_sdl_get_calling_bank, NULL);


    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__sprite_attrib"), 2, ses_sdl_sprite_attrib, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__engine_attrib"), 3, ses_sdl_engine_attrib, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__palette_attrib"), 5, ses_sdl_palette_attrib, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__tile_attrib"), 3, ses_sdl_tile_attrib, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__input_attrib"), 3, ses_sdl_input_attrib, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__audio_attrib"), 4, ses_sdl_audio_attrib, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__bg_attrib"), 4, ses_sdl_bg_attrib, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__oscillator_attrib"), 2, ses_sdl_oscillator_attrib, NULL);


    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__palette_query"), 3, ses_sdl_palette_query, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "ses_native__tile_query"), 2, ses_sdl_tile_query, NULL);


    ses.mainContext = context_create();
    //sdl.aux = context_create();    

   
}

void ses_native_swap_context() {
    //SES_Context c = sdl.main;
    //sdl.main = sdl.aux;
    //sdl.aux = c;
    //sdl.swapped = !sdl.swapped;
}


static void push_motion_callback() {
    matteHeap_t * heap = matte_vm_get_heap(ses.vm);
    uint32_t i;
    uint32_t len = matte_array_get_size(ses.mainContext.inputs[SES_DEVICE__POINTER0].callbacks);
    if (len == 0) return;
   
    double xcon, ycon;
    int w, h;
    ses_window_get_size(ses.window, &w, &h); 
    
    matteString_t * xStr = (matteString_t*)MATTE_VM_STR_CAST(ses.vm, "x");
    matteString_t * yStr = (matteString_t*)MATTE_VM_STR_CAST(ses.vm, "y");
    matteString_t * eventStr = (matteString_t*)MATTE_VM_STR_CAST(ses.vm, "event");
    
    
    matteValue_t x = matte_heap_new_value(heap);
    matteValue_t y = matte_heap_new_value(heap);
    matteValue_t event = matte_heap_new_value(heap);
    
    matte_value_into_string(heap, &x, xStr);
    matte_value_into_string(heap, &y, yStr);
    matte_value_into_string(heap, &event, eventStr);
    
    
    matteValue_t xval = matte_heap_new_value(heap);
    matteValue_t yval = matte_heap_new_value(heap);
    matteValue_t eventVal = matte_heap_new_value(heap);                
    
    int wr, hr;
    ses_graphics_context_get_render_size(ses.graphics, &w, &h);
    
    matte_value_into_number(heap, &xval, (ses.pointerX / (float) w) * wr);
    matte_value_into_number(heap, &yval, (ses.pointerY / (float) h) * hr);
    matte_value_into_number(heap, &eventVal, 0);



    matteValue_t namesArr[] = {event, x, y};
    matteValue_t valsArr[] = {eventVal, xval, yval};                
    
    for(i = 0; i < len; ++i) {
        matteValue_t val = matte_array_at(ses.mainContext.inputs[SES_DEVICE__POINTER0].callbacks, matteValue_t, i);    
        if (val.binID == 0) continue;

        // for safety
        matteArray_t names = MATTE_ARRAY_CAST(namesArr, matteValue_t, 3);
        matteArray_t vals = MATTE_ARRAY_CAST(valsArr, matteValue_t, 3);

        matte_vm_call(ses.vm, val, &vals, &names, NULL);
        
    }
}


static void push_scroll_callback() {
    matteHeap_t * heap = matte_vm_get_heap(ses.vm);
    uint32_t i;
    uint32_t len = matte_array_get_size(ses.mainContext.inputs[SES_DEVICE__POINTER0].callbacks);
    if (len == 0) return;

    matteString_t * xStr = (matteString_t*)MATTE_VM_STR_CAST(ses.vm, "x");
    matteString_t * yStr = (matteString_t*)MATTE_VM_STR_CAST(ses.vm, "y");
    matteString_t * eventStr = (matteString_t*)MATTE_VM_STR_CAST(ses.vm, "event");
    
    
    matteValue_t x = matte_heap_new_value(heap);
    matteValue_t y = matte_heap_new_value(heap);
    matteValue_t event = matte_heap_new_value(heap);
    
    matte_value_into_string(heap, &x, xStr);
    matte_value_into_string(heap, &y, yStr);
    matte_value_into_string(heap, &event, eventStr);
    
    
    matteValue_t xval = matte_heap_new_value(heap);
    matteValue_t yval = matte_heap_new_value(heap);
    matteValue_t eventVal = matte_heap_new_value(heap);
    
    
    
    
    matte_value_into_number(heap, &xval, ses.pointerScrollX);
    matte_value_into_number(heap, &yval, ses.pointerScrollY);
    matte_value_into_number(heap, &eventVal, 5); // scroll



    matteValue_t namesArr[] = {event, x, y};
    matteValue_t valsArr[] = {eventVal, xval, yval};                
    
    for(i = 0; i < len; ++i) {
        matteValue_t val = matte_array_at(ses.mainContext.inputs[SES_DEVICE__POINTER0].callbacks, matteValue_t, i);    
        if (val.binID == 0) continue;

        // for safety
        matteArray_t names = MATTE_ARRAY_CAST(namesArr, matteValue_t, 3);
        matteArray_t vals = MATTE_ARRAY_CAST(valsArr, matteValue_t, 3);

        matte_vm_call(ses.vm, val, &vals, &names, NULL);
        
    }

}





static void ses_native_update__key_callback(
    sesWindow_t * window, 
    sesWindow_Event_t eventID, 
    void * eventData, 
    void * userData    
) {

    matteHeap_t * heap = matte_vm_get_heap(ses.vm);
    sesWindow_Event_Key_t * evt = eventData;
    
    uint32_t i;
    uint32_t len = matte_array_get_size(ses.mainContext.inputs[SES_DEVICE__KEYBOARD].callbacks);
    if (len == 0) return;
    
    matteString_t * textStr = (matteString_t*)MATTE_VM_STR_CAST(ses.vm, "key");
    matteString_t * eventStr = (matteString_t*)MATTE_VM_STR_CAST(ses.vm, "event");
    
    
    matteValue_t text = matte_heap_new_value(heap);
    matteValue_t event = matte_heap_new_value(heap);
    
    matte_value_into_string(heap, &text, textStr);
    matte_value_into_string(heap, &event, eventStr);
    
    
    matteValue_t textval = matte_heap_new_value(heap);
    matteValue_t eventVal = matte_heap_new_value(heap);
    
    double xcon, ycon;
    int w, h;
    
    
    matte_value_into_number(heap, &textval, evt->key);
    matte_value_into_number(heap, &eventVal, (evt->down ? 2 : 6));// key down



    matteValue_t namesArr[] = {event, text};
    matteValue_t valsArr[] = {eventVal, textval};                
    
    for(i = 0; i < len; ++i) {
        matteValue_t val = matte_array_at(ses.mainContext.inputs[SES_DEVICE__KEYBOARD].callbacks, matteValue_t, i);    
        if (val.binID == 0) continue;

        // for safety
        matteArray_t names = MATTE_ARRAY_CAST(namesArr, matteValue_t, 2);
        matteArray_t vals = MATTE_ARRAY_CAST(valsArr, matteValue_t, 2);

        matte_vm_call(ses.vm, val, &vals, &names, NULL);
        
    }   
}


static void ses_native_update__text_callback(
    sesWindow_t * window, 
    sesWindow_Event_t eventID, 
    void * eventData, 
    void * userData    
) {

    matteHeap_t * heap = matte_vm_get_heap(ses.vm);
    sesWindow_Event_Text_t * evt = eventData;

    uint32_t i;
    uint32_t len = matte_array_get_size(ses.mainContext.inputs[SES_DEVICE__KEYBOARD].callbacks);
    if (len == 0) return;
    
    matteString_t * textStr = (matteString_t*)MATTE_VM_STR_CAST(ses.vm, "text");
    matteString_t * eventStr = (matteString_t*)MATTE_VM_STR_CAST(ses.vm, "event");
    
    
    matteValue_t text = matte_heap_new_value(heap);
    matteValue_t event = matte_heap_new_value(heap);
    
    matte_value_into_string(heap, &text, textStr);
    matte_value_into_string(heap, &event, eventStr);
    
    
    matteValue_t textval = matte_heap_new_value(heap);
    matteValue_t eventVal = matte_heap_new_value(heap);
    
    double xcon, ycon;
    int w, h;
    
    
    matteString_t * textRaw = matte_string_create_from_c_str("%s", evt->text);
    matte_value_into_string(heap, &textval, textRaw);
    matte_string_destroy(textRaw);
    matte_value_into_number(heap, &eventVal, 1);



    matteValue_t namesArr[] = {event, text};
    matteValue_t valsArr[] = {eventVal, textval};                
    
    for(i = 0; i < len; ++i) {
        matteValue_t val = matte_array_at(ses.mainContext.inputs[SES_DEVICE__KEYBOARD].callbacks, matteValue_t, i);    
        if (val.binID == 0) continue;

        // for safety
        matteArray_t names = MATTE_ARRAY_CAST(namesArr, matteValue_t, 2);
        matteArray_t vals = MATTE_ARRAY_CAST(valsArr, matteValue_t, 2);

        matte_vm_call(ses.vm, val, &vals, &names, NULL);
        
    }   


}



static void ses_native_update__pointer_button_callback(
    sesWindow_t * window, 
    sesWindow_Event_t eventID, 
    void * eventData, 
    void * userData    
) {

    matteHeap_t * heap = matte_vm_get_heap(ses.vm);
    sesWindow_Event_Pointer_t * evt = eventData;

    uint32_t i;
    uint32_t len = matte_array_get_size(ses.mainContext.inputs[SES_DEVICE__POINTER0].callbacks);
    if (len == 0) return;
    
    matteString_t * xStr = (matteString_t*)MATTE_VM_STR_CAST(ses.vm, "x");
    matteString_t * yStr = (matteString_t*)MATTE_VM_STR_CAST(ses.vm, "y");
    matteString_t * buttonStr = (matteString_t*)MATTE_VM_STR_CAST(ses.vm, "button");
    matteString_t * eventStr = (matteString_t*)MATTE_VM_STR_CAST(ses.vm, "event");
    
    
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
    ses_window_get_size(ses.window, &w, &h);
    

    int wr, hr;
    ses_graphics_context_get_render_size(ses.graphics, &w, &h);
    
    
    matte_value_into_number(heap, &xval, (evt->x / (float) w) * wr);
    matte_value_into_number(heap, &yval, (evt->y / (float) h) * hr);
    matte_value_into_number(heap, &eventVal, evt->down ? 3 : 4);

    int which;
    switch(evt->button) {
      case SES_WINDOW_EVENT__POINTER_BUTTON__LEFT:   which = 0; break;
      case SES_WINDOW_EVENT__POINTER_BUTTON__MIDDLE: which = 1; break;
      case SES_WINDOW_EVENT__POINTER_BUTTON__RIGHT:  which = 2; break;
    }

    matte_value_into_number(heap, &buttonval, evt->button);



    matteValue_t namesArr[] = {event, x, y, button};
    matteValue_t valsArr[] = {eventVal, xval, yval, buttonval};                
    
    for(i = 0; i < len; ++i) {
        matteValue_t val = matte_array_at(ses.mainContext.inputs[SES_DEVICE__POINTER0].callbacks, matteValue_t, i);    
        if (val.binID == 0) continue;

        // for safety
        matteArray_t names = MATTE_ARRAY_CAST(namesArr, matteValue_t, 4);
        matteArray_t vals = MATTE_ARRAY_CAST(valsArr, matteValue_t, 4);

        matte_vm_call(ses.vm, val, &vals, &names, NULL);
        
    }    
}


static void ses_native_update__pointer_scroll_callback(
    sesWindow_t * window, 
    sesWindow_Event_t eventID, 
    void * eventData, 
    void * userData    
) {

    matteHeap_t * heap = matte_vm_get_heap(ses.vm);
    sesWindow_Event_Pointer_t * evt = eventData;

    ses.hasPointerScrollEvent = 1;            
    ses.pointerScrollX = evt->x;
    ses.pointerScrollY = evt->y;
}



static void ses_native_update__pointer_motion_callback(
    sesWindow_t * window, 
    sesWindow_Event_t eventID, 
    void * eventData, 
    void * userData    
) {

    matteHeap_t * heap = matte_vm_get_heap(ses.vm);
    sesWindow_Event_Pointer_t * evt = eventData;

    ses.hasPointerMotionEvent = 1;
    ses.pointerX = evt->x;
    ses.pointerY = evt->y;
}


static void ses_native_update__frame_render_callback(
    sesWindow_t * window, 
    sesWindow_Event_t eventID, 
    void * eventData, 
    void * userData    
) {

    matteHeap_t * heap = matte_vm_get_heap(ses.vm);
    sesWindow_Event_Pointer_t * evt = eventData;

    if (ses.mainContext.updateFunc.binID) {
        matte_vm_call(
            ses.vm,
            ses.mainContext.updateFunc,
            matte_array_empty(),
            matte_array_empty(),
            NULL
        );
    }

    int i, len;

    // only process motion / scroll events on frame render
    // this groups them logically rather than clogging up the even queue.
    if (ses.hasPointerMotionEvent) {
        ses.hasPointerMotionEvent = 0;
        push_motion_callback();
    }
    if (ses.hasPointerScrollEvent) {
        ses.hasPointerScrollEvent = 0;
        push_scroll_callback();
    }

    ses_native_render();


}

int ses_native_update(matte_t * m) {
    ses.vm = matte_get_vm(m);
    matteHeap_t * heap = matte_vm_get_heap(ses.vm);
    ses_window_resolve_events(ses.window);
    ses_cartridge_poll_oscillators(ses.mainCart, ses_window_get_ticks(ses.window));
    ses_window_thread_wait(ses.window, 1);
   
    return 1;
}


int ses_native_main_loop(matte_t * m) {
    ses_window_set_event_callback(ses.window, SES_WINDOW_EVENT__KEY,            ses_native_update__key_callback, NULL);
    ses_window_set_event_callback(ses.window, SES_WINDOW_EVENT__TEXT,           ses_native_update__text_callback, NULL);
    ses_window_set_event_callback(ses.window, SES_WINDOW_EVENT__POINTER_BUTTON, ses_native_update__pointer_button_callback, NULL);
    ses_window_set_event_callback(ses.window, SES_WINDOW_EVENT__POINTER_SCROLL, ses_native_update__pointer_scroll_callback, NULL);
    ses_window_set_event_callback(ses.window, SES_WINDOW_EVENT__POINTER_MOTION, ses_native_update__pointer_motion_callback, NULL);
    ses_window_set_event_callback(ses.window, SES_WINDOW_EVENT__FRAME_RENDER,   ses_native_update__frame_render_callback, NULL);


    while(ses_native_update(m)) {}
    return 0;
}

/*
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
    
) {
    SES_Context * ctx = sdl.swapped ? &sdl.aux : &sdl.main;
    if (index >= SPRITE_COUNT_TOTAL) return 0;
    
    SES_Sprite * spr = &ctx->sprites[index];
    *x = spr->x;
    *y = spr->y;
    *rotation = spr->rotation;
    *scaleX = spr->scaleX;
    *scaleY = spr->scaleY;
    *centerX = spr->centerX;
    *centerY = spr->centerY;
    *layer = spr->layer;
    *effect = spr->effect;
    *enabled = spr->enabled;
    *palette = spr->palette;
    *tile = spr->tile;
    return 1;
}


int ses_native_get_tile_info(
    uint32_t tile,
    uint8_t * data
) {
    if (tile > TILE_ID_MAX) return 0;
    ses_sdl_gl_bind_tile(tile);
    
    int i;
    for(i = 0; i < 64; ++i) {
        data[i] = ses_sdl_gl_get_tile_pixel(i);
    }
    
    ses_sdl_gl_unbind_tile(tile);
    return 1;    
}


int ses_native_get_palette_info(
    uint32_t index,
    sesVector_t * data
) {
    SES_Context * ctx = sdl.swapped ? &sdl.aux : &sdl.main;
    if (index >= matte_array_get_size(ctx->palettes)) return 0;

    SES_Palette p = matte_array_at(ctx->palettes, SES_Palette, index);
    data[0] = p.back;
    data[1] = p.midBack;
    data[2] = p.midFront;
    data[3] = p.front;
    return 1;
}

static matteValue_t ses_sdl_get_calling_bank(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteValue_t v = matte_heap_new_value(matte_vm_get_heap(vm));
    matte_value_into_number(matte_vm_get_heap(vm), &v, 0);
    return v;
}
*/


