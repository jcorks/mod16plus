#include "cartridge.h"
#include "matte/src/matte_heap.h"
#include "matte/src/matte_vm.h"
#include "matte/src/matte_bytecode_stub.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    // all oscillators
    mod16Cartridge_Oscillator_t all[MOD16_CARTRIDGE__MAX_OSCILLATOR_COUNT];
    
    // array of active oscillators
    mod16Cartridge_Oscillator_t * active[MOD16_CARTRIDGE__MAX_OSCILLATOR_COUNT];
    
    // number of active oscillators;
    uint32_t activeCount;

} mod16Cartridge_OscillatorContext_t;

struct mod16Cartridge_t {
    matteVM_t * vm;
    
    // graphics context used for all operations.
    // parents and children always use the same graphics
    // context.
    mod16GraphicsContext_t * graphics;


    // Storage for rendering operations. This is unique to 
    // each cartridge.
    mod16GraphicsContext_Storage_t * storage;
    
    // The rom for the cart. If parent != NULL, the rom is owned 
    // by the cartridge and needs to be destroyed with the cart
    mod16ROM_t * rom;

    // an array of MOD16_Sprite, representing all accessed 
    // sprites.
    mod16GraphicsContext_Sprite_t sprites[MOD16_CARTRIDGE__MAX_SPRITE_COUNT];   

    mod16GraphicsContext_Background_t bgs[MOD16_CARTRIDGE__MAX_BACKGROUND_COUNT];


    // all oscillators
    mod16Cartridge_OscillatorContext_t osc;
    
    // linked list of active sprites
    // This ref is the head (prev == NULL)
    mod16GraphicsContext_Sprite_t * activeSprites;
    
    
    // The result of the main script after boot. Used as 
    // the API for working with that cartridge.
    // Only toplevel carts will reliably have no return value.
    matteValue_t main;
    
    // The parent cartridge if any. Toplevel carts will have this 
    // set to NULL
    mod16Cartridge_t * parent;
    
    // Child cartridges. References are owned by the parent.
    matteArray_t * children;

    // one matteValue_t for each source in the rom.
    matteValue_t * sourceValues;

    // flag for whether each source was run.
    uint8_t * sourcesRun;
    
    // THe unique ID of the cart. Increments
    int uniqueID;
    
    
    // The name of this rom
    matteString_t * name;
    
    // the fullname of the rom including the ancestry as prefixes
    matteString_t * fullname;
};

static matteArray_t * ALL_CARTRIDGES = NULL;

mod16Cartridge_t * mod16_cartridge_create(matteVM_t * vm, mod16ROM_t * rom, mod16GraphicsContext_t * graphics, const matteString_t * namein, mod16Cartridge_t * parent) {
    if (!ALL_CARTRIDGES) {
        ALL_CARTRIDGES = matte_array_create(sizeof(mod16Cartridge_t *));
    }
    mod16Cartridge_t * cart = calloc(1, sizeof(mod16Cartridge_t));
    cart->vm = vm;
    cart->graphics = graphics;
    cart->rom = rom;
    cart->storage =  mod16_graphics_context_create_storage(graphics);
    cart->main = matte_heap_new_value(matte_vm_get_heap(vm));   
    cart->children = matte_array_create(sizeof(mod16Cartridge_t *));
    cart->uniqueID = matte_array_get_size(ALL_CARTRIDGES);
    cart->sourceValues = calloc(mod16_rom_get_bytecode_segment_count(rom), sizeof(matteValue_t));
    cart->sourcesRun = calloc(mod16_rom_get_bytecode_segment_count(rom), 1);
    cart->name = matte_string_clone(namein);   
    cart->parent = parent;
    if (parent == NULL) {      
        cart->fullname = matte_string_clone(namein);
    } else {
        cart->fullname = matte_string_create_from_c_str("%s/%s", matte_string_get_c_str(parent->fullname), matte_string_get_c_str(cart->name));            

    }    
    printf("Registered Cartridge: %s\n", matte_string_get_c_str(cart->fullname));
    
    
    matte_array_push(ALL_CARTRIDGES, cart);
    
    int i;
    for(i = 0; i < MOD16_CARTRIDGE__MAX_SPRITE_COUNT; ++i) {
        cart->sprites[i].scaleX = 1;
        cart->sprites[i].scaleY = 1;
    }
    
    
    matteString_t * name = matte_string_create();
    for(i = 0; i < mod16_rom_get_subcartridge_rom_count(rom); ++i) {
        uint32_t bytelen;
        const uint8_t * bytes = mod16_rom_get_subcartridge_rom(
            rom,
            i,
            &bytelen, 
            name
        );
        mod16ROM_UnpackError_t result;
        mod16ROM_t * subrom = mod16_rom_unpack(bytes, bytelen, &result); 
        if (result != 0) {
            printf("Unpacking sub-ROM resulted in error:\n");
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
            break;
        }
        
        if (subrom) {
            mod16Cartridge_t * subcart = mod16_cartridge_create(vm, subrom, graphics, name, cart);
            matte_array_push(cart->children, subcart);
        }
    }
    matte_string_destroy(name);

    return cart;
}


matteValue_t mod16_cartridge_get_main(mod16Cartridge_t * cart) {
    return cart->main;
}

mod16ROM_t * mod16_cartridge_get_rom(const mod16Cartridge_t * cart) {
    return cart->rom;
}


// Gets the context storage for this 
mod16GraphicsContext_Storage_t * mod16_cartridge_get_context_storage(mod16Cartridge_t * cart) {
    return cart->storage;
}



static int COUNT_ENABLED = 0;
void mod16_cartridge_enable_sprite(mod16Cartridge_t * cart, uint16_t index, int enabled) {
    if (index >= MOD16_CARTRIDGE__MAX_SPRITE_COUNT) return;
    
    mod16GraphicsContext_Sprite_t * spr = &cart->sprites[index];

    if (spr->enabled == enabled) return;;
    
    
    spr->enabled = enabled;
    if (enabled) {

        // take from inactive list and place as new head 
        // to main active list.
        spr->prev = NULL;
        spr->next = cart->activeSprites;
        if (spr->next) {
            spr->next->prev = spr;
        }
        cart->activeSprites = spr;
        COUNT_ENABLED++;
        printf("ENABLED:::%d\n", COUNT_ENABLED);
    } else {
        if (spr == cart->activeSprites) {
            if (cart->activeSprites->next)
                cart->activeSprites->next->prev = NULL;
            cart->activeSprites = spr->next;
        } else {
            if (spr->next) {
                spr->next->prev = spr->prev;
            }
            if (spr->prev) {
                spr->prev->next = spr->next;
            }
        }
        COUNT_ENABLED--;
        printf("DISABLED:::%d\n", COUNT_ENABLED);
    }
}


mod16GraphicsContext_Sprite_t * mod16_cartridge_get_sprite(mod16Cartridge_t * cart, uint16_t index) {
    if (index >= MOD16_CARTRIDGE__MAX_SPRITE_COUNT) return NULL;
    return &cart->sprites[index];
}

mod16GraphicsContext_Background_t * mod16_cartridge_get_background(mod16Cartridge_t * cart, uint16_t index) {
    if (index >= MOD16_CARTRIDGE__MAX_BACKGROUND_COUNT) return NULL;
    return &cart->bgs[index];
}


mod16Cartridge_Oscillator_t * mod16_cartridge_get_oscillator(mod16Cartridge_t * cart, uint16_t index) {
    if (index >= MOD16_CARTRIDGE__MAX_OSCILLATOR_COUNT) return NULL;
    return &cart->osc.all[index];
}


void mod16_cartridge_enable_oscillator(mod16Cartridge_t * cart, uint16_t index, int enabled, double ticks) {
    if (index >= MOD16_CARTRIDGE__MAX_OSCILLATOR_COUNT) return;
    mod16Cartridge_Oscillator_t * osc = &cart->osc.all[index];

    matteHeap_t * heap = matte_vm_get_heap(cart->vm);
    int n;
    if (osc->active && enabled) {
        // resets the oscillator when re-enabled
        osc->startMS = ticks;
        osc->endMS = osc->lengthMS + osc->startMS;                
    } else if (enabled != osc->active) {
        osc->active = enabled;            
        if (osc->active) {
            cart->osc.active[cart->osc.activeCount++] = osc;
            osc->startMS = ticks;
            osc->endMS = osc->lengthMS + osc->startMS;
        } else {
            int found = 0;
            for(n = 0; n < cart->osc.activeCount-1; ++n) {
                if (cart->osc.active[n] == osc) {
                    found = 1;
                } 
                
                if (found) {
                    cart->osc.active[n] = cart->osc.active[n+1];
                }
            }
            cart->osc.activeCount--;
        }
    }
}


// Updates
void mod16_cartridge_poll_oscillators(mod16Cartridge_t * cart, double ticks) {
    uint32_t i;
    uint32_t len = matte_array_get_size(cart->children);
    for(i = 0; i < len; ++i) {
        mod16Cartridge_t * subcart = matte_array_at(cart->children, mod16Cartridge_t *, i);
        mod16_cartridge_poll_oscillators(subcart, ticks);
    }


    len = cart->osc.activeCount;
    for(i = 0; i < len; ++i) {
        mod16Cartridge_Oscillator_t * alarm = cart->osc.active[i];
        if (ticks >= alarm->endMS) {
            if (alarm->function.binID) {
                matte_vm_call(
                    cart->vm,
                    alarm->function,
                    matte_array_empty(),
                    matte_array_empty(),
                    NULL
                );
            }

            alarm->startMS = ticks;
            alarm->endMS = ticks + alarm->lengthMS;
        }
    }    
}




mod16Cartridge_t * mod16_cartridge_push_graphics(mod16Cartridge_t * cart, mod16GraphicsContext_t * ctx) {
    uint32_t i;
    uint32_t len = matte_array_get_size(cart->children);
    for(i = 0; i < len; ++i) {
        mod16Cartridge_t * subcart = matte_array_at(cart->children, mod16Cartridge_t *, i);
        mod16_cartridge_push_graphics(subcart, ctx);
    }
    

    mod16GraphicsContext_Sprite_t * iter = cart->activeSprites;
    while(iter) {
        mod16_graphics_context_add_sprite(ctx, iter, cart->storage);
        iter = iter->next;
    }



    

    int n;
    for(i = 0; i < MOD16_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_COUNT; ++i) {
        mod16GraphicsContext_Background_t * bg = &cart->bgs[i];
        if (bg->enabled == 0) continue;


        mod16_graphics_context_add_background(ctx, bg, cart->storage);
    }
}



static matteValue_t compile_run_bytes(const matteString_t * name, matteVM_t * vm, const uint8_t * bytes, uint32_t len) {

    
    uint32_t fileid = matte_vm_get_new_file_id(vm, name);
    
    matteArray_t * stubs = matte_bytecode_stubs_from_bytecode(
        matte_vm_get_heap(vm),
        fileid,
        bytes,
        len
    );
    
    matte_vm_add_stubs(vm, stubs);
    return matte_vm_run_fileid(vm, fileid, matte_heap_new_value(matte_vm_get_heap(vm)), MATTE_VM_STR_CAST(vm,""));
}



mod16Cartridge_t * mod16_cartridge_get_subcartridge(mod16Cartridge_t * cart, const matteString_t * name) {
    uint32_t i;
    uint32_t len = matte_array_get_size(cart->children);
    matteString_t * test = matte_string_create();
    for(i = 0; i < len; ++i) {
        uint32_t bytelen;
        const uint8_t * bytes = mod16_rom_get_subcartridge_rom(
            cart->rom,
            i,
            &bytelen, 
            test
        );        
    
        if (matte_string_test_eq(name, test)) {
            matte_string_destroy(test);
            return matte_array_at(cart->children, mod16Cartridge_t *, i);
        }
    }
    matte_string_destroy(test);
    return NULL;
}







static matteArray_t * ACTIVE_CARTS = NULL;


void mod16_cartridge_bootup(mod16Cartridge_t * cart) {
    if (!ACTIVE_CARTS) {
        ACTIVE_CARTS = matte_array_create(sizeof(mod16Cartridge_t *));    
    }

    // children are ALWAYS booted first
    uint32_t i;
    uint32_t len = matte_array_get_size(cart->children);
    for(i = 0; i < len; ++i) {
        mod16Cartridge_t * subcart = matte_array_at(cart->children, mod16Cartridge_t *, i);
        mod16_cartridge_bootup(subcart);
    }


    printf("Initializing Cartridge: %s\n", matte_string_get_c_str(cart->fullname));

    matte_array_push(ACTIVE_CARTS, cart);
    cart->main = mod16_cartridge_get_source(cart, MATTE_VM_STR_CAST(cart->vm, "main"));
    matte_array_set_size(ACTIVE_CARTS, matte_array_get_size(ACTIVE_CARTS)-1);
}


mod16Cartridge_t * mod16_cartridge_get_active_boot_context() {
    if (!ACTIVE_CARTS) return NULL;
    if (matte_array_get_size(ACTIVE_CARTS) == 0) return NULL;
    return matte_array_at(ACTIVE_CARTS, mod16Cartridge_t *, matte_array_get_size(ACTIVE_CARTS)-1);
}

uint32_t mod16_cartridge_get_id(const mod16Cartridge_t * cart) {
    return cart->uniqueID;
}

matteValue_t mod16_cartridge_get_source(mod16Cartridge_t * cart, const matteString_t * source) {
    uint32_t id;
    int found = 0;
    uint32_t i;
    uint32_t len = mod16_rom_get_bytecode_segment_count(cart->rom);
    uint32_t byteLen;
    const uint8_t * bytes;
    matteString_t * name = matte_string_create();
    for(i = 0; i < len; ++i) {
        bytes = mod16_rom_get_bytecode_segment(cart->rom, i, &byteLen, name);
        
        if (matte_string_test_eq(name, source)) {
            found = 1;
            id = i;
            break;
        }
    }
    matte_string_destroy(name);
    if (!found) {
        matte_vm_raise_error_string(cart->vm, MATTE_VM_STR_CAST(cart->vm, "Could not get source: does not exist"));
        return matte_heap_new_value(matte_vm_get_heap(cart->vm));
    }
    
    if (!cart->sourcesRun[id]) {
        matteString_t * fname = matte_string_create_from_c_str(
            "%s/%s",
            matte_string_get_c_str(cart->fullname),
            matte_string_get_c_str(source)
        );
        cart->sourceValues[id] = compile_run_bytes(
            fname,
            cart->vm,
            bytes,
            byteLen
        );   
        matte_string_destroy(fname);
        // for safety
        matte_value_object_push_lock(matte_vm_get_heap(cart->vm), cart->sourceValues[id]);
        cart->sourcesRun[id] = 1;
    }
    return cart->sourceValues[id];

}


mod16Cartridge_t * mod16_cartridge_from_id(uint32_t id) {
    if (!ALL_CARTRIDGES) return NULL;
    if (id >= matte_array_get_size(ALL_CARTRIDGES)) return NULL;
    return matte_array_at(ALL_CARTRIDGES, mod16Cartridge_t *, id);
}


const matteString_t * mod16_cartridge_get_name(mod16Cartridge_t * cart) {
    return cart->name;
}

const matteString_t * mod16_cartridge_get_fullname(mod16Cartridge_t * cart) {
    return cart->fullname;
}




