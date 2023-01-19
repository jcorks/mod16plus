#include "cartridge.h"
#include <stdlib.h>


typedef struct {
    // all oscillators
    sesCartridge_Oscillator_t all[SES_CARTRIDGE__MAX_OSCILLATOR_COUNT];
    
    // array of active oscillators
    sesCartridge_Oscillator_t * active[SES_CARTRIDGE__MAX_OSCILLATOR_COUNT];
    
    // number of active oscillators;
    uint32_t activeCount;

} sesCartridge_OscillatorContext_t;

struct sesCartridge_t {
    sesGraphicsContext_Storage_t * storage;

    // an array of SES_Sprite, representing all accessed 
    // sprites.
    sesGraphicsContext_Sprite_t sprites[SES_CARTRIDGE__MAX_SPRITE_COUNT];   


    // all oscillators
    sesCartridge_OscillatorContext_t osc;
    
    // linked list of active sprites
    // This ref is the head (prev == NULL)
    sesGraphicsContext_Sprite_t * activeSprites;
};




void ses_cartridge_enable_sprite(sesCartridge_t * cart, uint16_t index, int enabled) {
    if (index >= SES_CARTRIDGE__MAX_SPRITE_COUNT) return;
    
    sesGraphicsContext_Sprite_t * spr = &cart->sprites[index];

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
    }
}


void ses_cartridge_get_sprite(sesCartridge_t * cart, uint16_t index) {
    if (index >= SES_CARTRIDGE__MAX_SPRITE_COUNT) return NULL;
    return &cart->sprites[index];
}




sesCartridge_Oscillator_t * ses_cartridge_get_oscillator(sesCartridge_t * cart, uint16_t index) {
    if (index >= SES_CARTRIDGE__MAX_OSCILLATOR_COUNT) return NULL;
    return &cart->osc.all[index];
}

void ses_cartridge_enable_oscillator(sesCartridge_t * cart, uint16_t index, int enabled) {
    if (index >= SES_CARTRIDGE__MAX_OSCILLATOR_COUNT) return;
    sesCartridge_Oscillator_t * osc = &cart->osc.all[index];

    if (osc->active && enabled) {
        // resets the oscillator when re-enabled
        osc->startMS = SDL_GetTicks();
        osc->endMS = osc->lengthMS + osc->startMS;                
    } else if (enabled != osc->active) {
        osc->active = matte_value_as_boolean(heap, *value);            
        if (osc->active) {
            sdl.main.osc.active[sdl.main.osc.activeCount++] = osc;
            osc->startMS = SDL_GetTicks();
            osc->endMS = osc->lengthMS + osc->startMS;
        } else {
            int found = 0;
            for(n = 0; n < sdl.main.osc.activeCount-1; ++n) {
                if (osc_get_index(sdl.main.osc.active[n]) == id) {
                    found = 1;
                } 
                
                if (found) {
                    sdl.main.osc.active[n] = sdl.main.osc.active[n+1];
                }
            }
            sdl.main.osc.activeCount--;
        }
    }
}


// Updates
void ses_cartridge_poll_oscillators(sesCartridge_t * cart, double ticks) {
    int i;
    int len = sdl.main.osc.activeCount;
    for(i = 0; i < len; ++i) {
        sesCartridge_OscillatorContext_t * alarm = sdl.main.osc.active[i];
        if (ticks >= alarm->endMS) {
            if (alarm->function.binID) {
                matte_vm_call(
                    sdl.vm,
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




sesCartridge_t * ses_cartridge_push_graphics(sesCartridge_t * cart, sesGraphicsContext_t * ctx) {
    sesGraphicsContext_Sprite_t * iter = cart->activeSprites;
    while(iter) {
        ses_graphics_context_add_sprite(ctx, iter, cart->storage);
        iter = iter->next;
    }



    sesGraphicsContext_Background_t * bg = matte_array_get_data(cart->bgs);
    int i, n;
    int len = matte_array_get_size(sdl.main.bgs);
    for(i = 0; i < len; ++i, bg++) {
        if (bg->enabled == 0) continue;


        ses_graphics_context_add_background(ctx, bg, cart->storage);
    }
}


