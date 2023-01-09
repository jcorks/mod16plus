#include "cartridge.h"



typedef struct {
    // all oscillators
    SES_Oscillator all[SES_CARTRIDGE__MAX_OSCILLATOR_COUNT];
    
    // array of active oscillators
    SES_Oscillator * active[SES_CARTRIDGE__MAX_OSCILLATOR_COUNT];
    
    // number of active oscillators;
    uint32_t activeCount;

} sesCartridge_OscillatorContext_t;

struct sesCartridge_t {

    // an array of SES_Sprite, representing all accessed 
    // sprites.
    SES_Sprite sprites[SES_CARTRIDGE__MAX_SPRITE_COUNT];   


    // all oscillators
    sesCartridge_OscillatorContext_t osc;
    
    // linked list of active sprites
    // This ref is the head (prev == NULL)
    SES_Sprite * activeSprites;
};


void ses_cartridge_enable_sprite(sesCartridge_t * cart, uint16_t index, int enabled) {
    if (index >= SES_CARTRIDGE__MAX_SPRITE_COUNT) return;
    
    sesCartridge_Sprite_t * spr = &cart->sprites[index];

    int enabled = matte_value_as_number(heap, *value);
    if (spr->enabled == enabled) break;
    
    
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
        SES_Oscillator * alarm = sdl.main.osc.active[i];
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
    sesCartridge_Sprite_t * iter = cart->activeSprites;
    while(iter) {
        sesGraphicsContext_Layer_t * layer = ses_graphics_context_get_layer(ctx, iter->layer-(LAYER_MIN));
        ses_graphics_context_layer_add_sprite(layer, iter);
        iter = iter->next;
    }



    sesCartridge_Background_t * bg = matte_array_get_data(cart->bgs);
    int i, n;
    int len = matte_array_get_size(sdl.main.bgs);
    for(i = 0; i < len; ++i, bg++) {
        if (bg->enabled == 0) continue;


        sesGraphicsContext_Layer_t * layer = ses_graphics_context_get_layer(ctx, bg->layer-(LAYER_MIN));
        ses_graphics_context_layer_add_background(layer->bgs, bg);
    }
}


