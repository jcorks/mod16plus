#include "matte/src/matte_vm.h"
#include "matte/src/matte.h"
#include "matte/src/matte_array.h"
#include "matte/src/matte_string.h"
#include <stdio.h>
#include "native.h"
#include "window.h"
#include "cartridge.h"
#include <math.h>
#include <string.h>
#define SPRITE_MAX 4096
#define OSCILLATOR_MAX 1024
#define SPRITE_COUNT_TOTAL 65536
#define TILE_ID_MAX 0x8ffff


typedef enum {
    MOD16_DEVICE__KEYBOARD,
    
    MOD16_DEVICE__POINTER0,
    MOD16_DEVICE__POINTER1,
    MOD16_DEVICE__POINTER2,
    MOD16_DEVICE__POINTER3,

    MOD16_DEVICE__GAMEPAD0,
    MOD16_DEVICE__GAMEPAD1,
    MOD16_DEVICE__GAMEPAD2,
    MOD16_DEVICE__GAMEPAD3,

    
} MOD16_DeviceType;











typedef struct {
    // matteValue_t of functions to call
    matteArray_t * callbacks;

    // uint32_t IDs waiting to be used.
    matteArray_t * dead;
} MOD16_InputCallbackSet;







typedef struct {
    MOD16_InputCallbackSet inputs[MOD16_DEVICE__GAMEPAD3+1];

    // user function called every frame    
    matteValue_t updateFunc;


} mod16Context_t;





typedef struct {
    // native window
    mod16Window_t * window;

    mod16GraphicsContext_t * graphics;
    
    matteVM_t * vm;



    mod16Context_t mainContext;
    mod16Context_t auxContext;

    // every cartridge has a 
    mod16Cartridge_t * mainCart;
    
    // whether the main and aux are swapped. Usually for debugging context
    int swapped;


    // current pointer
    int hasPointerMotionEvent;
    int hasPointerScrollEvent;


    int pointerX;
    int pointerY;
    int pointerScrollX;
    int pointerScrollY;    

} mod16Native_t;

static mod16Native_t mod16 = {};



static matteValue_t mod16_native_get_context_cartridge_id(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);


static matteValue_t mod16_native_sprite_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
static matteValue_t mod16_native_engine_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
static matteValue_t mod16_native_palette_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
static matteValue_t mod16_native_tile_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
static matteValue_t mod16_native_input_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
static matteValue_t mod16_native_audio_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
static matteValue_t mod16_native_bg_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);

static matteValue_t mod16_native_palette_query(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);
static matteValue_t mod16_native_tile_query(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData);








static void mod16_native_render() {
    // re-sort sprites and bgs into layer buckets;
    mod16_cartridge_push_graphics(mod16.mainCart, mod16.graphics);
    mod16_graphics_context_render(mod16.graphics);
}










mod16GraphicsContext_Sprite_t * mod16_native_bind_sprite(matteVM_t * vm, matteHeap_t * heap, const matteValue_t * args) {

    uint32_t cartID = matte_value_as_number(heap, args[0]);
    mod16Cartridge_t * cart = mod16_cartridge_from_id(cartID);


    uint32_t id = matte_value_as_number(heap, args[1]);
    mod16GraphicsContext_Sprite_t * spr = mod16_cartridge_get_sprite(cart, id);
    if (!spr) {
        matte_vm_raise_error_string(vm, MATTE_VM_STR_CAST(vm, "Sprite accessed beyond limit"));                
        return NULL;   
    }
    return spr;
}



matteValue_t mod16_native_sprite_attrib__rotation(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16GraphicsContext_Sprite_t * spr = mod16_native_bind_sprite(vm, heap, args);
    spr->rotation = matte_value_as_number(heap, args[2]);
    return matte_heap_new_value(heap);
}

matteValue_t mod16_native_sprite_attrib__scaleX(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16GraphicsContext_Sprite_t * spr = mod16_native_bind_sprite(vm, heap, args);
    spr->scaleX = matte_value_as_number(heap, args[2]);
    return matte_heap_new_value(heap);
}

matteValue_t mod16_native_sprite_attrib__scaleY(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16GraphicsContext_Sprite_t * spr = mod16_native_bind_sprite(vm, heap, args);
    spr->scaleY = matte_value_as_number(heap, args[2]);
    return matte_heap_new_value(heap);
}

matteValue_t mod16_native_sprite_attrib__positionx(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16GraphicsContext_Sprite_t * spr = mod16_native_bind_sprite(vm, heap, args);
    spr->x = matte_value_as_number(heap, args[2]);
    return matte_heap_new_value(heap);
}


matteValue_t mod16_native_sprite_attrib__positiony(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16GraphicsContext_Sprite_t * spr = mod16_native_bind_sprite(vm, heap, args);
    spr->y = matte_value_as_number(heap, args[2]);
    return matte_heap_new_value(heap);
}

matteValue_t mod16_native_sprite_attrib__centerx(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16GraphicsContext_Sprite_t * spr = mod16_native_bind_sprite(vm, heap, args);
    spr->centerX = matte_value_as_number(heap, args[2]);
    return matte_heap_new_value(heap);
}


matteValue_t mod16_native_sprite_attrib__centery(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16GraphicsContext_Sprite_t * spr = mod16_native_bind_sprite(vm, heap, args);
    spr->centerY = matte_value_as_number(heap, args[2]);
    return matte_heap_new_value(heap);
}

matteValue_t mod16_native_sprite_attrib__layer(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16GraphicsContext_Sprite_t * spr = mod16_native_bind_sprite(vm, heap, args);
    spr->layer = matte_value_as_number(heap, args[2]);
    if (spr->layer > MOD16_GRAPHICS_CONTEXT__LAYER_MAX) spr->layer = MOD16_GRAPHICS_CONTEXT__LAYER_MAX;
    if (spr->layer < MOD16_GRAPHICS_CONTEXT__LAYER_MIN) spr->layer = MOD16_GRAPHICS_CONTEXT__LAYER_MIN;
    return matte_heap_new_value(heap);
}

matteValue_t mod16_native_sprite_attrib__tile(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16GraphicsContext_Sprite_t * spr = mod16_native_bind_sprite(vm, heap, args);
    spr->tile = matte_value_as_number(heap, args[2]);
    return matte_heap_new_value(heap);
}

matteValue_t mod16_native_sprite_attrib__effect(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16GraphicsContext_Sprite_t * spr = mod16_native_bind_sprite(vm, heap, args);
    spr->effect = matte_value_as_number(heap, args[2]);
    return matte_heap_new_value(heap);
}

matteValue_t mod16_native_sprite_attrib__palette(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16GraphicsContext_Sprite_t * spr = mod16_native_bind_sprite(vm, heap, args);
    spr->palette = matte_value_as_number(heap, args[2]);
    return matte_heap_new_value(heap);
}








matteValue_t mod16_native_sprite_attrib__show(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    
    uint32_t cartID = matte_value_as_number(heap, args[0]);
    mod16Cartridge_t * cart = mod16_cartridge_from_id(cartID);
    uint32_t id = matte_value_as_number(heap, args[1]);

    mod16_cartridge_enable_sprite(cart, id, matte_value_as_number(heap, args[2]));
    return matte_heap_new_value(heap);
}









mod16Cartridge_Oscillator_t * mod16_native_bind_osc(matteVM_t * vm, matteHeap_t * heap, const matteValue_t * args) {

    uint32_t cartID = matte_value_as_number(heap, args[0]);
    mod16Cartridge_t * cart = mod16_cartridge_from_id(cartID);

    uint32_t id = matte_value_as_number(heap, args[1]);
    mod16Cartridge_Oscillator_t * osc = mod16_cartridge_get_oscillator(cart, id);
    
    if (!osc) {
        matte_vm_raise_error_string(vm, MATTE_VM_STR_CAST(vm, "Oscillator accessed beyond limit"));
        return NULL;
    }
    return osc;
}



matteValue_t mod16_native_oscillator_attrib__enable(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16Cartridge_Oscillator_t * spr = mod16_native_bind_osc(vm, heap, args);
    uint32_t cartID = matte_value_as_number(heap, args[0]);
    mod16Cartridge_t * cart = mod16_cartridge_from_id(cartID);
    uint32_t id = matte_value_as_number(heap, args[1]);
    mod16_cartridge_enable_oscillator(cart, id, matte_value_as_boolean(heap, args[2]), mod16_window_get_ticks(mod16.window));
    return matte_heap_new_value(heap);
}



matteValue_t mod16_native_oscillator_attrib__periodms(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16Cartridge_Oscillator_t * osc = mod16_native_bind_osc(vm, heap, args);
    osc->lengthMS = matte_value_as_number(heap, args[2]);
    osc->endMS = osc->startMS + osc->lengthMS;
    return matte_heap_new_value(heap);
}


matteValue_t mod16_native_oscillator_attrib__oncycle(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16Cartridge_Oscillator_t * osc = mod16_native_bind_osc(vm, heap, args);

    if (osc->function.value.id == args[2].value.id) return matte_heap_new_value(heap);
    if (osc->function.binID) {
        matte_value_object_pop_lock(heap, osc->function);
    }
    osc->function = args[2];
    matte_value_object_push_lock(heap, osc->function);
    return matte_heap_new_value(heap);
}



matteValue_t mod16_native_oscillator_attrib__time(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16Cartridge_Oscillator_t * osc = mod16_native_bind_osc(vm, heap, args);
    float prog = (osc->endMS - mod16_window_get_ticks(mod16.window)) / (double)osc->lengthMS;
    double frac = 0.5*(1+sin((prog) * (2*M_PI)));
    matteValue_t fracVal = matte_heap_new_value(heap);
    matte_value_into_number(heap, &fracVal, frac);
    return fracVal;
}





matteValue_t mod16_native_vertices_set_count(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);

    uint32_t cartID = matte_value_as_number(heap, args[0]);
    mod16Cartridge_t * cart = mod16_cartridge_from_id(cartID);

    
    mod16GraphicsContext_Storage_t * storage = mod16_cartridge_get_context_storage(cart);
    mod16_graphics_context_storage_set_vertex_count(
        storage, 
        matte_value_as_number(heap, args[1])
    );        
    
    return matte_heap_new_value(heap);
}



matteValue_t mod16_native_vertices_set_shape(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);

    uint32_t cartID = matte_value_as_number(heap, args[0]);
    mod16Cartridge_t * cart = mod16_cartridge_from_id(cartID);

    
    mod16GraphicsContext_VertexSettings_t * settings = mod16_cartridge_get_vertex_settings(cart);
    settings->shape = matte_value_as_number(heap, args[1]);
    
    return matte_heap_new_value(heap);
}


matteValue_t mod16_native_vertices_set_transform(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);

    uint32_t cartID = matte_value_as_number(heap, args[0]);
    mod16Cartridge_t * cart = mod16_cartridge_from_id(cartID);

    mod16GraphicsContext_VertexSettings_t * settings = mod16_cartridge_get_vertex_settings(cart);
    mod16Matrix_t * tf = &settings->transform;    
    matteValue_t array = args[1];
    int i;
    matteValue_t value;
    for(i = 0; i < 16; ++i) {
        matteValue_t v = matte_value_object_access_index(heap, array, i);
        tf->data[i] = matte_value_as_number(heap, v);
    }
    
    return matte_heap_new_value(heap);
}


matteValue_t mod16_native_vertices_set_effect(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);

    uint32_t cartID = matte_value_as_number(heap, args[0]);
    mod16Cartridge_t * cart = mod16_cartridge_from_id(cartID);

    
    mod16GraphicsContext_VertexSettings_t * settings = mod16_cartridge_get_vertex_settings(cart);
    settings->effect = matte_value_as_number(heap, args[1]);
    
    return matte_heap_new_value(heap);
}

matteValue_t mod16_native_vertices_set_layer(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);

    uint32_t cartID = matte_value_as_number(heap, args[0]);
    mod16Cartridge_t * cart = mod16_cartridge_from_id(cartID);

    
    mod16GraphicsContext_VertexSettings_t * settings = mod16_cartridge_get_vertex_settings(cart);
    settings->layer = matte_value_as_number(heap, args[1]);
    
    return matte_heap_new_value(heap);
}


matteValue_t mod16_native_vertices_set_palette(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);

    uint32_t cartID = matte_value_as_number(heap, args[0]);
    mod16Cartridge_t * cart = mod16_cartridge_from_id(cartID);

    
    mod16GraphicsContext_VertexSettings_t * settings = mod16_cartridge_get_vertex_settings(cart);
    settings->palette = matte_value_as_number(heap, args[1]);
    
    return matte_heap_new_value(heap);
}


matteValue_t mod16_native_vertices_set_textured(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);

    uint32_t cartID = matte_value_as_number(heap, args[0]);
    mod16Cartridge_t * cart = mod16_cartridge_from_id(cartID);

    
    mod16GraphicsContext_VertexSettings_t * settings = mod16_cartridge_get_vertex_settings(cart);
    settings->textured = matte_value_as_number(heap, args[1]);
    
    return matte_heap_new_value(heap);
}


matteValue_t mod16_native_vertices_set(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);

    uint32_t cartID = matte_value_as_number(heap, args[0]);
    mod16Cartridge_t * cart = mod16_cartridge_from_id(cartID);
    

    mod16GraphicsContext_Storage_t * storage = mod16_cartridge_get_context_storage(cart);
    uint16_t index = matte_value_as_number(heap, args[1]);

    mod16GraphicsContext_Vertex_t vertex = {};
    vertex.x = matte_value_as_number(heap, matte_value_object_access_index(heap, args[2], 0));
    vertex.y = matte_value_as_number(heap, matte_value_object_access_index(heap, args[2], 1));
    vertex.z = matte_value_as_number(heap, matte_value_object_access_index(heap, args[2], 2));
    vertex.r = matte_value_as_number(heap, matte_value_object_access_index(heap, args[2], 3));
    vertex.g = matte_value_as_number(heap, matte_value_object_access_index(heap, args[2], 4));
    vertex.b = matte_value_as_number(heap, matte_value_object_access_index(heap, args[2], 5));
    vertex.u = matte_value_as_number(heap, matte_value_object_access_index(heap, args[2], 6));
    vertex.v = matte_value_as_number(heap, matte_value_object_access_index(heap, args[2], 7));
    vertex.tile = matte_value_as_number(heap, matte_value_object_access_index(heap, args[2], 8));


    mod16_graphics_context_storage_set_vertex(storage, index, &vertex);    
    return matte_heap_new_value(heap);
}


matteValue_t mod16_native_vertices_get(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);

    uint32_t cartID = matte_value_as_number(heap, args[0]);
    mod16Cartridge_t * cart = mod16_cartridge_from_id(cartID);
    

    mod16GraphicsContext_Storage_t * storage = mod16_cartridge_get_context_storage(cart);
    uint16_t index = matte_value_as_number(heap, args[1]);

    matteArray_t * arr = matte_array_create(sizeof(matteValue_t));    
    matteValue_t m;
    
    const mod16GraphicsContext_Vertex_t * v = mod16_graphics_context_storage_get_vertex(storage, index);    

    m = matte_heap_new_value(heap); matte_value_into_number(heap, &m, v->x); matte_array_push(arr, m);
    m = matte_heap_new_value(heap); matte_value_into_number(heap, &m, v->y); matte_array_push(arr, m);
    m = matte_heap_new_value(heap); matte_value_into_number(heap, &m, v->z); matte_array_push(arr, m);
    m = matte_heap_new_value(heap); matte_value_into_number(heap, &m, v->r); matte_array_push(arr, m);
    m = matte_heap_new_value(heap); matte_value_into_number(heap, &m, v->g); matte_array_push(arr, m);
    m = matte_heap_new_value(heap); matte_value_into_number(heap, &m, v->b); matte_array_push(arr, m);
    m = matte_heap_new_value(heap); matte_value_into_number(heap, &m, v->u); matte_array_push(arr, m);
    m = matte_heap_new_value(heap); matte_value_into_number(heap, &m, v->v); matte_array_push(arr, m);
    m = matte_heap_new_value(heap); matte_value_into_number(heap, &m, v->tile); matte_array_push(arr, m);




    matteValue_t out;
    matte_value_into_new_object_array_ref(heap, &out, arr);
    matte_array_destroy(arr);

    return out;
}







typedef enum {
    MOD16NEA_UPDATERATE,
    MOD16NEA_UPDATEFUNC,
    MOD16NEA_CLIPBOARDGET,
    MOD16NEA_CLIPBOARDSET 
} MOD16Native_EngineAttribs_t;



matteValue_t mod16_native_engine_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    printf("ENGINE   ID: %d\n",
        (int)matte_value_as_number(heap, args[0])
    );  
    
    switch((int)matte_value_as_number(heap, args[0])) {

      // function to call to update each frame, according to the user.
      case MOD16NEA_UPDATEFUNC:
        matte_value_object_pop_lock(heap, mod16.mainContext.updateFunc);
        mod16.mainContext.updateFunc = args[1];
        matte_value_object_push_lock(heap, mod16.mainContext.updateFunc);
        break;
        
      case MOD16NEA_UPDATERATE:
        mod16_window_set_frame_update_delay(mod16.window, matte_value_as_number(heap, args[1]) * 1000);
        break;


      case MOD16NEA_CLIPBOARDGET: {
        matteValue_t strOut = matte_heap_new_value(heap);
        matteString_t * strVal = mod16_window_get_clipboard(mod16.window);
        matte_value_into_string(heap, &strOut, strVal);
        matte_string_destroy(strVal);

        return strOut;
      };

      case MOD16NEA_CLIPBOARDSET: {
        const matteString_t * str = matte_value_string_get_string_unsafe(heap, args[1]);
        mod16_window_set_clipboard(mod16.window, str);
      };



        
    }
    
    return matte_heap_new_value(heap);
}



typedef enum {
    MOD16NPA_BACK,
    MOD16NPA_MIDBACK,
    MOD16NPA_MIDFRONT,
    MOD16NPA_FRONT
} MOD16Native_PaletteAttribs_t;

matteValue_t mod16_native_palette_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);

    uint32_t cartID = matte_value_as_number(heap, args[0]);
    mod16Cartridge_t * cart = mod16_cartridge_from_id(cartID);
    
    uint32_t id = matte_value_as_number(heap, args[1]);
    const mod16GraphicsContext_Palette_t * pin = mod16_graphics_context_storage_get_palette(
        mod16_cartridge_get_context_storage(
            cart
        ),
        id
    );
    if (!pin) {
        matte_vm_raise_error_string(vm, MATTE_VM_STR_CAST(vm, "Palette accessed beyond limit"));
        return matte_heap_new_value(heap);
    }
    mod16GraphicsContext_Palette_t p = *pin;


    switch((int)matte_value_as_number(heap, args[2])) {
      case MOD16NPA_BACK:
        p.back.x = matte_value_as_number(heap, args[3]);
        p.back.y = matte_value_as_number(heap, args[4]);
        p.back.z = matte_value_as_number(heap, args[5]);
        break;

      case MOD16NPA_MIDBACK:
        p.midBack.x = matte_value_as_number(heap, args[3]);
        p.midBack.y = matte_value_as_number(heap, args[4]);
        p.midBack.z = matte_value_as_number(heap, args[5]);
        break;

      case MOD16NPA_MIDFRONT:
        p.midFront.x = matte_value_as_number(heap, args[3]);
        p.midFront.y = matte_value_as_number(heap, args[4]);
        p.midFront.z = matte_value_as_number(heap, args[5]);
        break;

      case MOD16NPA_FRONT:
        p.front.x = matte_value_as_number(heap, args[3]);
        p.front.y = matte_value_as_number(heap, args[4]);
        p.front.z = matte_value_as_number(heap, args[5]);
        break;

            
    }
    
    
    mod16_graphics_context_storage_set_palette(
        mod16_cartridge_get_context_storage(
            cart
        ),
        id,
        &p
    );
    return matte_heap_new_value(heap);
}

typedef enum {
    MOD16NTA_SET,
    MOD16NTA_COPY
} MOD16Native_TileAttribs_t;


matteValue_t mod16_native_tile_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);

    uint32_t cartID = matte_value_as_number(heap, args[0]);
    mod16Cartridge_t * cart = mod16_cartridge_from_id(cartID);


    uint32_t id = matte_value_as_number(heap, args[1]);
    const mod16GraphicsContext_Tile_t * tilep = mod16_graphics_context_storage_get_tile(
        mod16_cartridge_get_context_storage(
            cart
        ),
        id    
    );
    
    if (!tilep) {
        matte_vm_raise_error_string(vm, MATTE_VM_STR_CAST(vm, "Tile accessed beyond limit"));
        return matte_heap_new_value(heap);    
    }

    mod16GraphicsContext_Tile_t tile = *tilep;


    switch((int)matte_value_as_number(heap, args[2])) {
        
      case MOD16NTA_SET: {
        matteValue_t array = args[3];
        int pixelCount = MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS*MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS;
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
        
        mod16_graphics_context_storage_set_tile(
            mod16_cartridge_get_context_storage(
                cart
            ),
            id,
            &tile            
        );
        
        break;
      }
        
      case MOD16NTA_COPY: {
        uint32_t tid = matte_value_as_number(heap, args[3]);
        const mod16GraphicsContext_Tile_t * tilet = mod16_graphics_context_storage_get_tile(
            mod16_cartridge_get_context_storage(
                cart
            ),
            tid    
        );
        if (!tilet) {
            matte_vm_raise_error_string(vm, MATTE_VM_STR_CAST(vm, "Tile (target of copy) accessed beyond limit"));
            return matte_heap_new_value(heap);    
        }
        
        mod16_graphics_context_storage_set_tile(
            mod16_cartridge_get_context_storage(
                cart
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
    MOD16NIA_ADD,
    MOD16NIA_REMOVE
} MOD16Native_InputAction_t;

matteValue_t mod16_native_input_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    printf("INPUT    ID: %d, ATTRIB: %d\n",
        (int)matte_value_as_number(heap, args[0]),
        (int)matte_value_as_number(heap, args[1])
    );  

    switch((int)matte_value_as_number(heap, args[0])) {
      case MOD16NIA_ADD: {
        MOD16_InputCallbackSet * set = &mod16.mainContext.inputs[(int)matte_value_as_number(heap, args[1])];

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
      
      
      case MOD16NIA_REMOVE: {
        MOD16_InputCallbackSet * set = &mod16.mainContext.inputs[(int)matte_value_as_number(heap, args[1])];
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
matteValue_t mod16_native_audio_attrib(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    return matte_heap_new_value(heap);
}



matteValue_t mod16_native_get_context_cartridge_id(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {

    matteValue_t v = matte_heap_new_value(matte_vm_get_heap(vm));
    matte_value_into_number(
        matte_vm_get_heap(vm), 
        &v, 
        mod16_cartridge_get_id(
            mod16_cartridge_get_active_boot_context()
        )    
    );
    return v;

}

matteValue_t mod16_native_has_boot_context(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {

    matteValue_t v = matte_heap_new_value(matte_vm_get_heap(vm));
    matte_value_into_boolean(
        matte_vm_get_heap(vm), 
        &v, 
        mod16_cartridge_get_active_boot_context() != NULL
    );
    return v;

}


matteValue_t mod16_native_get_source(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    uint32_t id = matte_value_as_number(matte_vm_get_heap(vm), args[0]);
    const matteString_t * name = matte_value_string_get_string_unsafe(matte_vm_get_heap(vm), args[1]);

    mod16Cartridge_t * cart = mod16_cartridge_from_id(id);
    
    
    matteValue_t out = matte_heap_new_value(matte_vm_get_heap(vm));
    if (!cart) return out;
    return mod16_cartridge_get_source(cart, name);
}




matteValue_t mod16_native_get_sub_cartridge_main(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    uint32_t id = matte_value_as_number(matte_vm_get_heap(vm), args[0]);
    const matteString_t * name = matte_value_string_get_string_unsafe(matte_vm_get_heap(vm), args[1]);

    mod16Cartridge_t * cart = mod16_cartridge_get_subcartridge(
        mod16_cartridge_from_id(id),
        name
    );
    
    
    matteValue_t out = matte_heap_new_value(matte_vm_get_heap(vm));
    if (!cart) return out;
    return mod16_cartridge_get_main(cart);
}



mod16GraphicsContext_Background_t * mod16_native_bind_bg(matteVM_t * vm, matteHeap_t * heap, const matteValue_t * args) {

    uint32_t cartID = matte_value_as_number(heap, args[0]);
    mod16Cartridge_t * cart = mod16_cartridge_from_id(cartID);


    uint32_t id = matte_value_as_number(heap, args[1]);
    mod16GraphicsContext_Background_t * bg = mod16_cartridge_get_background(cart, id);
    
    if (!bg) {
        matte_vm_raise_error_string(vm, MATTE_VM_STR_CAST(vm, "BG accessed beyond limit"));                
        return NULL;;    
    }
    return bg;
}



matteValue_t mod16_native_bg_attrib__rotation(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16GraphicsContext_Background_t * spr = mod16_native_bind_bg(vm, heap, args);
    spr->rotation = matte_value_as_number(heap, args[2]);
    return matte_heap_new_value(heap);
}

matteValue_t mod16_native_bg_attrib__scaleX(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16GraphicsContext_Background_t * spr = mod16_native_bind_bg(vm, heap, args);
    spr->scaleX = matte_value_as_number(heap, args[2]);
    return matte_heap_new_value(heap);
}

matteValue_t mod16_native_bg_attrib__scaleY(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16GraphicsContext_Background_t * spr = mod16_native_bind_bg(vm, heap, args);
    spr->scaleY = matte_value_as_number(heap, args[2]);
    return matte_heap_new_value(heap);
}

matteValue_t mod16_native_bg_attrib__positionx(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16GraphicsContext_Background_t * spr = mod16_native_bind_bg(vm, heap, args);
    spr->x = matte_value_as_number(heap, args[2]);
    return matte_heap_new_value(heap);
}


matteValue_t mod16_native_bg_attrib__positiony(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16GraphicsContext_Background_t * spr = mod16_native_bind_bg(vm, heap, args);
    spr->y = matte_value_as_number(heap, args[2]);
    return matte_heap_new_value(heap);
}

matteValue_t mod16_native_bg_attrib__centerx(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16GraphicsContext_Background_t * spr = mod16_native_bind_bg(vm, heap, args);
    spr->centerX = matte_value_as_number(heap, args[2]);
    return matte_heap_new_value(heap);
}


matteValue_t mod16_native_bg_attrib__centery(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16GraphicsContext_Background_t * spr = mod16_native_bind_bg(vm, heap, args);
    spr->centerY = matte_value_as_number(heap, args[2]);
    return matte_heap_new_value(heap);
}

matteValue_t mod16_native_bg_attrib__layer(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16GraphicsContext_Background_t * spr = mod16_native_bind_bg(vm, heap, args);
    spr->layer = matte_value_as_number(heap, args[2]);
    if (spr->layer > MOD16_GRAPHICS_CONTEXT__LAYER_MAX) spr->layer = MOD16_GRAPHICS_CONTEXT__LAYER_MAX;
    if (spr->layer < MOD16_GRAPHICS_CONTEXT__LAYER_MIN) spr->layer = MOD16_GRAPHICS_CONTEXT__LAYER_MIN;
    return matte_heap_new_value(heap);
}



matteValue_t mod16_native_bg_attrib__effect(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16GraphicsContext_Background_t * spr = mod16_native_bind_bg(vm, heap, args);
    spr->effect = matte_value_as_number(heap, args[2]);
    return matte_heap_new_value(heap);
}

matteValue_t mod16_native_bg_attrib__palette(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16GraphicsContext_Background_t * spr = mod16_native_bind_bg(vm, heap, args);
    spr->palette = matte_value_as_number(heap, args[2]);
    return matte_heap_new_value(heap);
}

matteValue_t mod16_native_bg_attrib__show(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    mod16GraphicsContext_Background_t * spr = mod16_native_bind_bg(vm, heap, args);
    spr->enabled = matte_value_as_number(heap, args[2]);
    return matte_heap_new_value(heap);
}











matteValue_t mod16_native_palette_query(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    printf("P.QUERY  ID: %d, ATTRIB: %d\n",
        (int)matte_value_as_number(heap, args[0]),
        (int)matte_value_as_number(heap, args[1])
    );  
    uint32_t cartID = matte_value_as_number(heap, args[0]);
    mod16Cartridge_t * cart = mod16_cartridge_from_id(cartID);
    
    uint32_t id = matte_value_as_number(heap, args[1]);
    const mod16GraphicsContext_Palette_t * p = mod16_graphics_context_storage_get_palette(
        mod16_cartridge_get_context_storage(
            cart
        ),
        id
    );
    if (!p) {
        matte_vm_raise_error_string(vm, MATTE_VM_STR_CAST(vm, "Palette accessed beyond limit"));
        return matte_heap_new_value(heap);
    }   

    const mod16Vector_t * color;
    switch((int)matte_value_as_number(heap, args[2])) {
      case MOD16NPA_BACK:
        color = &p->back;
        break;

      case MOD16NPA_MIDBACK:
        color = &p->midBack;
        break;

      case MOD16NPA_MIDFRONT:
        color = &p->midFront;
        break;

      case MOD16NPA_FRONT:
        color = &p->front;
        break;

      default:    return matte_heap_new_value(heap);
    }
    
    matteValue_t out = matte_heap_new_value(heap);
    
    switch((int)matte_value_as_number(heap, args[3])) {
      case 0: matte_value_into_number(heap, &out, color->x); break;
      case 1: matte_value_into_number(heap, &out, color->y); break;
      case 2: matte_value_into_number(heap, &out, color->z); break;
    }
    
    return out;
    
    

}
matteValue_t mod16_native_tile_query(matteVM_t * vm, matteValue_t fn, const matteValue_t * args, void * userData) {
    matteHeap_t * heap = matte_vm_get_heap(vm);
    matteValue_t out = matte_heap_new_value(heap);

    uint32_t cartID = matte_value_as_number(heap, args[0]);
    mod16Cartridge_t * cart = mod16_cartridge_from_id(cartID);

    uint32_t id = matte_value_as_number(heap, args[1]);
    const mod16GraphicsContext_Tile_t * tilep = mod16_graphics_context_storage_get_tile(
        mod16_cartridge_get_context_storage(
            cart
        ),
        id    
    );

    if (tilep == NULL)
        return out;    
    
    int i;
    int count = MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS * MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS;
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




static mod16Context_t context_create() {
    mod16Context_t ctx = {};
    /*
    MOD16_Context ctx = {};

    ctx.bgs = matte_array_create(sizeof(MOD16_Background));
    ctx.palettes = matte_array_create(sizeof(MOD16_Palette));
    ctx.sprites = calloc(1, sizeof(MOD16_Sprite) * SPRITE_COUNT_TOTAL);
    int i;
    for(i = 0; i < SPRITE_COUNT_TOTAL; ++i) {
        ctx.sprites[i].scaleX = 1;
        ctx.sprites[i].scaleY = 1;
    }
    */
    int i;
    for(i = 0; i <= MOD16_DEVICE__GAMEPAD3; ++i) {
        ctx.inputs[i].callbacks = matte_array_create(sizeof(matteValue_t));    
        ctx.inputs[i].dead = matte_array_create(sizeof(uint32_t));
    }
    

    /*
    for(i = 0; i < 128; ++i) {
        MOD16_GraphicsLayer * layer = &ctx.layers[i];
        layer->sprites = matte_array_create(sizeof(MOD16_Sprite *));
        layer->bgs = matte_array_create(sizeof(MOD16_Background *));
    }
    return ctx;
    */
    return ctx;
}

void mod16_native_commit_rom(mod16ROM_t * rom, matte_t * m) {
    matteVM_t * vm = matte_get_vm(m);
    mod16.window = mod16_window_create();
    mod16.graphics = mod16_window_get_graphics(mod16.window);
    mod16.mainCart = mod16_cartridge_create(vm, rom, mod16.graphics, MATTE_VM_STR_CAST(vm, "ROM"), NULL);

    // all 3 modes require activating the core features.
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__get_context_cartridge_id"), 0, mod16_native_get_context_cartridge_id, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__has_boot_context"), 0, mod16_native_has_boot_context, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__get_source"), 2, mod16_native_get_source, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__get_sub_cartridge_main"), 2, mod16_native_get_sub_cartridge_main, NULL);



    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__sprite_attrib__centerx"), 3, mod16_native_sprite_attrib__centerx, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__sprite_attrib__centery"), 3, mod16_native_sprite_attrib__centery, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__sprite_attrib__effect"), 3, mod16_native_sprite_attrib__effect, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__sprite_attrib__layer"), 3, mod16_native_sprite_attrib__layer, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__sprite_attrib__palette"), 3, mod16_native_sprite_attrib__palette, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__sprite_attrib__positionx"), 3, mod16_native_sprite_attrib__positionx, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__sprite_attrib__positiony"), 3, mod16_native_sprite_attrib__positiony, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__sprite_attrib__rotation"), 3, mod16_native_sprite_attrib__rotation, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__sprite_attrib__scaleX"), 3, mod16_native_sprite_attrib__scaleX, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__sprite_attrib__scaleY"), 3, mod16_native_sprite_attrib__scaleY, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__sprite_attrib__show"), 3, mod16_native_sprite_attrib__show, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__sprite_attrib__tile"), 3, mod16_native_sprite_attrib__tile, NULL);


    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__bg_attrib__centerx"), 3, mod16_native_bg_attrib__centerx, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__bg_attrib__centery"), 3, mod16_native_bg_attrib__centery, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__bg_attrib__effect"), 3, mod16_native_bg_attrib__effect, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__bg_attrib__layer"), 3, mod16_native_bg_attrib__layer, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__bg_attrib__palette"), 3, mod16_native_bg_attrib__palette, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__bg_attrib__positionx"), 3, mod16_native_bg_attrib__positionx, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__bg_attrib__positiony"), 3, mod16_native_bg_attrib__positiony, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__bg_attrib__rotation"), 3, mod16_native_bg_attrib__rotation, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__bg_attrib__scaleX"), 3, mod16_native_bg_attrib__scaleX, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__bg_attrib__scaleY"), 3, mod16_native_bg_attrib__scaleY, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__bg_attrib__show"), 3, mod16_native_bg_attrib__show, NULL);

    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__oscillator_attrib__enable"), 3, mod16_native_oscillator_attrib__enable, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__oscillator_attrib__periodms"), 3, mod16_native_oscillator_attrib__periodms, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__oscillator_attrib__oncycle"), 3, mod16_native_oscillator_attrib__oncycle, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__oscillator_attrib__time"), 2, mod16_native_oscillator_attrib__time, NULL);





    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__engine_attrib"), 4, mod16_native_engine_attrib, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__palette_attrib"), 6, mod16_native_palette_attrib, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__tile_attrib"), 4, mod16_native_tile_attrib, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__input_attrib"), 4, mod16_native_input_attrib, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__audio_attrib"), 5, mod16_native_audio_attrib, NULL);


    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__palette_query"), 4, mod16_native_palette_query, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__tile_query"), 3, mod16_native_tile_query, NULL);


    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__vertices_set_count"), 2, mod16_native_vertices_set_count, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__vertices_set_shape"), 2, mod16_native_vertices_set_shape, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__vertices_set_transform"), 2, mod16_native_vertices_set_transform, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__vertices_set_effect"), 2, mod16_native_vertices_set_effect, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__vertices_set_layer"), 2, mod16_native_vertices_set_layer, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__vertices_set_palette"), 2, mod16_native_vertices_set_palette, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__vertices_set_textured"), 2, mod16_native_vertices_set_textured, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__vertices_set"), 3, mod16_native_vertices_set, NULL);
    matte_vm_set_external_function_autoname(vm, MATTE_VM_STR_CAST(vm, "mod16_native__vertices_get"), 2, mod16_native_vertices_get, NULL);


    mod16.mainContext = context_create();
    mod16.auxContext = context_create();    

   
}

void mod16_native_swap_context() {
    mod16Context_t c = mod16.mainContext;
    mod16.mainContext = mod16.auxContext;
    mod16.auxContext = c;
    mod16.swapped = !mod16.swapped;
}


static void push_motion_callback() {
    matteHeap_t * heap = matte_vm_get_heap(mod16.vm);
    uint32_t i;
    uint32_t len = matte_array_get_size(mod16.mainContext.inputs[MOD16_DEVICE__POINTER0].callbacks);
    if (len == 0) return;
   
    double xcon, ycon;
    int w, h;
    mod16_window_get_size(mod16.window, &w, &h); 
    
    matteString_t * xStr = (matteString_t*)MATTE_VM_STR_CAST(mod16.vm, "x");
    matteString_t * yStr = (matteString_t*)MATTE_VM_STR_CAST(mod16.vm, "y");
    matteString_t * eventStr = (matteString_t*)MATTE_VM_STR_CAST(mod16.vm, "event");
    
    
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
    mod16_graphics_context_get_render_size(mod16.graphics, &wr, &hr);
    
    matte_value_into_number(heap, &xval, (mod16.pointerX / (float) w) * wr);
    matte_value_into_number(heap, &yval, (mod16.pointerY / (float) h) * hr);
    matte_value_into_number(heap, &eventVal, 0);



    matteValue_t namesArr[] = {event, x, y};
    matteValue_t valsArr[] = {eventVal, xval, yval};                
    
    for(i = 0; i < len; ++i) {
        matteValue_t val = matte_array_at(mod16.mainContext.inputs[MOD16_DEVICE__POINTER0].callbacks, matteValue_t, i);    
        if (val.binID == 0) continue;

        // for safety
        matteArray_t names = MATTE_ARRAY_CAST(namesArr, matteValue_t, 3);
        matteArray_t vals = MATTE_ARRAY_CAST(valsArr, matteValue_t, 3);

        matte_vm_call(mod16.vm, val, &vals, &names, NULL);
        
    }
}


static void push_scroll_callback() {
    matteHeap_t * heap = matte_vm_get_heap(mod16.vm);
    uint32_t i;
    uint32_t len = matte_array_get_size(mod16.mainContext.inputs[MOD16_DEVICE__POINTER0].callbacks);
    if (len == 0) return;

    matteString_t * xStr = (matteString_t*)MATTE_VM_STR_CAST(mod16.vm, "x");
    matteString_t * yStr = (matteString_t*)MATTE_VM_STR_CAST(mod16.vm, "y");
    matteString_t * eventStr = (matteString_t*)MATTE_VM_STR_CAST(mod16.vm, "event");
    
    
    matteValue_t x = matte_heap_new_value(heap);
    matteValue_t y = matte_heap_new_value(heap);
    matteValue_t event = matte_heap_new_value(heap);
    
    matte_value_into_string(heap, &x, xStr);
    matte_value_into_string(heap, &y, yStr);
    matte_value_into_string(heap, &event, eventStr);
    
    
    matteValue_t xval = matte_heap_new_value(heap);
    matteValue_t yval = matte_heap_new_value(heap);
    matteValue_t eventVal = matte_heap_new_value(heap);
    
    
    
    
    matte_value_into_number(heap, &xval, mod16.pointerScrollX);
    matte_value_into_number(heap, &yval, mod16.pointerScrollY);
    matte_value_into_number(heap, &eventVal, 5); // scroll



    matteValue_t namesArr[] = {event, x, y};
    matteValue_t valsArr[] = {eventVal, xval, yval};                
    
    for(i = 0; i < len; ++i) {
        matteValue_t val = matte_array_at(mod16.mainContext.inputs[MOD16_DEVICE__POINTER0].callbacks, matteValue_t, i);    
        if (val.binID == 0) continue;

        // for safety
        matteArray_t names = MATTE_ARRAY_CAST(namesArr, matteValue_t, 3);
        matteArray_t vals = MATTE_ARRAY_CAST(valsArr, matteValue_t, 3);

        matte_vm_call(mod16.vm, val, &vals, &names, NULL);
        
    }

}





static void mod16_native_update__key_callback(
    mod16Window_t * window, 
    mod16Window_Event_t eventID, 
    void * eventData, 
    void * userData    
) {

    matteHeap_t * heap = matte_vm_get_heap(mod16.vm);
    mod16Window_Event_Key_t * evt = eventData;
    
    uint32_t i;
    uint32_t len = matte_array_get_size(mod16.mainContext.inputs[MOD16_DEVICE__KEYBOARD].callbacks);
    if (len == 0) return;
    
    matteString_t * textStr = (matteString_t*)MATTE_VM_STR_CAST(mod16.vm, "key");
    matteString_t * eventStr = (matteString_t*)MATTE_VM_STR_CAST(mod16.vm, "event");
    
    
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
        matteValue_t val = matte_array_at(mod16.mainContext.inputs[MOD16_DEVICE__KEYBOARD].callbacks, matteValue_t, i);    
        if (val.binID == 0) continue;

        // for safety
        matteArray_t names = MATTE_ARRAY_CAST(namesArr, matteValue_t, 2);
        matteArray_t vals = MATTE_ARRAY_CAST(valsArr, matteValue_t, 2);

        matte_vm_call(mod16.vm, val, &vals, &names, NULL);
        
    }   
}


static void mod16_native_update__text_callback(
    mod16Window_t * window, 
    mod16Window_Event_t eventID, 
    void * eventData, 
    void * userData    
) {

    matteHeap_t * heap = matte_vm_get_heap(mod16.vm);
    mod16Window_Event_Text_t * evt = eventData;

    uint32_t i;
    uint32_t len = matte_array_get_size(mod16.mainContext.inputs[MOD16_DEVICE__KEYBOARD].callbacks);
    if (len == 0) return;
    
    matteString_t * textStr = (matteString_t*)MATTE_VM_STR_CAST(mod16.vm, "text");
    matteString_t * eventStr = (matteString_t*)MATTE_VM_STR_CAST(mod16.vm, "event");
    
    
    matteValue_t text = matte_heap_new_value(heap);
    matteValue_t event = matte_heap_new_value(heap);
    
    matte_value_into_string(heap, &text, textStr);
    matte_value_into_string(heap, &event, eventStr);
    
    
    matteValue_t textval = matte_heap_new_value(heap);
    matteValue_t eventVal = matte_heap_new_value(heap);
    
    double xcon, ycon;
    int w, h;
    
    
    matte_value_into_string(heap, &textval, evt->text);
    matte_value_into_number(heap, &eventVal, 1);



    matteValue_t namesArr[] = {event, text};
    matteValue_t valsArr[] = {eventVal, textval};                
    
    for(i = 0; i < len; ++i) {
        matteValue_t val = matte_array_at(mod16.mainContext.inputs[MOD16_DEVICE__KEYBOARD].callbacks, matteValue_t, i);    
        if (val.binID == 0) continue;

        // for safety
        matteArray_t names = MATTE_ARRAY_CAST(namesArr, matteValue_t, 2);
        matteArray_t vals = MATTE_ARRAY_CAST(valsArr, matteValue_t, 2);

        matte_vm_call(mod16.vm, val, &vals, &names, NULL);
        
    }   


}



static void mod16_native_update__pointer_button_callback(
    mod16Window_t * window, 
    mod16Window_Event_t eventID, 
    void * eventData, 
    void * userData    
) {

    matteHeap_t * heap = matte_vm_get_heap(mod16.vm);
    mod16Window_Event_Pointer_t * evt = eventData;

    uint32_t i;
    uint32_t len = matte_array_get_size(mod16.mainContext.inputs[MOD16_DEVICE__POINTER0].callbacks);
    if (len == 0) return;
    
    matteString_t * xStr = (matteString_t*)MATTE_VM_STR_CAST(mod16.vm, "x");
    matteString_t * yStr = (matteString_t*)MATTE_VM_STR_CAST(mod16.vm, "y");
    matteString_t * buttonStr = (matteString_t*)MATTE_VM_STR_CAST(mod16.vm, "button");
    matteString_t * eventStr = (matteString_t*)MATTE_VM_STR_CAST(mod16.vm, "event");
    
    
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
    mod16_window_get_size(mod16.window, &w, &h);
    

    int wr, hr;
    mod16_graphics_context_get_render_size(mod16.graphics, &wr, &hr);
    
    
    matte_value_into_number(heap, &xval, (evt->x / (float) w) * wr);
    matte_value_into_number(heap, &yval, (evt->y / (float) h) * hr);
    matte_value_into_number(heap, &eventVal, evt->down ? 3 : 4);

    int which;
    switch(evt->button) {
      case MOD16_WINDOW_EVENT__POINTER_BUTTON__LEFT:   which = 0; break;
      case MOD16_WINDOW_EVENT__POINTER_BUTTON__MIDDLE: which = 1; break;
      case MOD16_WINDOW_EVENT__POINTER_BUTTON__RIGHT:  which = 2; break;
    }

    matte_value_into_number(heap, &buttonval, evt->button);



    matteValue_t namesArr[] = {event, x, y, button};
    matteValue_t valsArr[] = {eventVal, xval, yval, buttonval};                
    
    for(i = 0; i < len; ++i) {
        matteValue_t val = matte_array_at(mod16.mainContext.inputs[MOD16_DEVICE__POINTER0].callbacks, matteValue_t, i);    
        if (val.binID == 0) continue;

        // for safety
        matteArray_t names = MATTE_ARRAY_CAST(namesArr, matteValue_t, 4);
        matteArray_t vals = MATTE_ARRAY_CAST(valsArr, matteValue_t, 4);

        matte_vm_call(mod16.vm, val, &vals, &names, NULL);
        
    }    
}


static void mod16_native_update__pointer_scroll_callback(
    mod16Window_t * window, 
    mod16Window_Event_t eventID, 
    void * eventData, 
    void * userData    
) {

    matteHeap_t * heap = matte_vm_get_heap(mod16.vm);
    mod16Window_Event_Pointer_t * evt = eventData;

    mod16.hasPointerScrollEvent = 1;            
    mod16.pointerScrollX = evt->x;
    mod16.pointerScrollY = evt->y;
}



static void mod16_native_update__pointer_motion_callback(
    mod16Window_t * window, 
    mod16Window_Event_t eventID, 
    void * eventData, 
    void * userData    
) {

    matteHeap_t * heap = matte_vm_get_heap(mod16.vm);
    mod16Window_Event_Pointer_t * evt = eventData;

    mod16.hasPointerMotionEvent = 1;
    mod16.pointerX = evt->x;
    mod16.pointerY = evt->y;
}


static void mod16_native_update__frame_render_callback(
    mod16Window_t * window, 
    mod16Window_Event_t eventID, 
    void * eventData, 
    void * userData    
) {

    matteHeap_t * heap = matte_vm_get_heap(mod16.vm);
    mod16Window_Event_Pointer_t * evt = eventData;

    if (mod16.mainContext.updateFunc.binID) {
        matte_vm_call(
            mod16.vm,
            mod16.mainContext.updateFunc,
            matte_array_empty(),
            matte_array_empty(),
            NULL
        );
    }

    int i, len;

    // only process motion / scroll events on frame render
    // this groups them logically rather than clogging up the even queue.
    if (mod16.hasPointerMotionEvent) {
        mod16.hasPointerMotionEvent = 0;
        push_motion_callback();
    }
    if (mod16.hasPointerScrollEvent) {
        mod16.hasPointerScrollEvent = 0;
        push_scroll_callback();
    }

    mod16_native_render();


}

int mod16_native_update(matte_t * m) {
    mod16.vm = matte_get_vm(m);
    matteHeap_t * heap = matte_vm_get_heap(mod16.vm);
    mod16_window_resolve_events(mod16.window);
    if (!mod16.swapped)
        mod16_cartridge_poll_oscillators(mod16.mainCart, mod16_window_get_ticks(mod16.window));
    mod16_window_thread_wait(mod16.window, 1);
   
    return 1;
}


int mod16_native_main_loop(matte_t * m) {
    mod16_window_set_event_callback(mod16.window, MOD16_WINDOW_EVENT__KEY,            mod16_native_update__key_callback, NULL);
    mod16_window_set_event_callback(mod16.window, MOD16_WINDOW_EVENT__TEXT,           mod16_native_update__text_callback, NULL);
    mod16_window_set_event_callback(mod16.window, MOD16_WINDOW_EVENT__POINTER_BUTTON, mod16_native_update__pointer_button_callback, NULL);
    mod16_window_set_event_callback(mod16.window, MOD16_WINDOW_EVENT__POINTER_SCROLL, mod16_native_update__pointer_scroll_callback, NULL);
    mod16_window_set_event_callback(mod16.window, MOD16_WINDOW_EVENT__POINTER_MOTION, mod16_native_update__pointer_motion_callback, NULL);
    mod16_window_set_event_callback(mod16.window, MOD16_WINDOW_EVENT__FRAME_RENDER,   mod16_native_update__frame_render_callback, NULL);


    mod16_cartridge_bootup(mod16.mainCart);    



    while(mod16_native_update(m)) {}
    return 0;
}



int mod16_native_get_palette_info(
    uint32_t index,
    mod16Vector_t * data
) {

    const mod16GraphicsContext_Palette_t * p = mod16_graphics_context_storage_get_palette(
        mod16_cartridge_get_context_storage(
            mod16.mainCart
        ),
        index
    );
    if (!p) {
        return 0;
    }   


    data[0] = p->back;
    data[1] = p->midBack;
    data[2] = p->midFront;
    data[3] = p->front;
    return 1;
}

int mod16_native_get_tile_info(
    uint32_t tile,
    uint8_t * data
) {
    const mod16GraphicsContext_Tile_t * tilep = mod16_graphics_context_storage_get_tile(
        mod16_cartridge_get_context_storage(
            mod16.mainCart
        ),
        tile
    );
    if (!tilep) {
        return 0;
    }   
    memcpy(data, tilep->data, MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS * MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS);

    return 1;    
}



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
    
) {
    mod16GraphicsContext_Sprite_t * spr = mod16_cartridge_get_sprite(mod16.mainCart, index);
    
    if (!spr) {
        return 0;
    }
    

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






