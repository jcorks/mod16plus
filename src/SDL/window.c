
#include "../window.h"
#include <SDL2/SDL.h>


struct mod16Window_t {
    SDL_Window    * window;

    mod16GraphicsContext_t * graphics;
    
    // delay before updating the next frame
    uint32_t frameUpdateDelayMS;
    
    // callback ID for the timer event emitter
    SDL_TimerID frameUpdateID;
    
    
    // all event callbacks
    mod16_window_event_callback_t callbacks[MOD16_WINDOW_EVENT__FRAME_RENDER+1];

    // corresponding data to the event.
    void * callbackData[MOD16_WINDOW_EVENT__FRAME_RENDER+1];
    
};

static uint32_t mod16_window_emit_frame_event(uint32_t interval, void * param) {
    
    SDL_Event event;
    SDL_UserEvent userevent = {};

    userevent.type = SDL_USEREVENT;
    event.type = SDL_USEREVENT;
    event.user = userevent;

    mod16Window_t * window = param;

    SDL_PushEvent(&event);    
    return window->frameUpdateDelayMS;
}




mod16Window_t * mod16_window_create() {
    if (SDL_Init(
        SDL_INIT_TIMER |
        SDL_INIT_VIDEO |
        SDL_INIT_AUDIO |
        SDL_INIT_GAMECONTROLLER            
    ) != 0) {
        printf("SDL2 subsystem init failure.\n");
        exit(1);
    }    
    
    mod16Window_t * out = calloc(1, sizeof(mod16Window_t));
    out->window  = SDL_CreateWindow("Mod16+", 0, 0, 240*4, 160*4, SDL_WINDOW_OPENGL);
    out->graphics = mod16_graphics_context_create(out);
    out->frameUpdateDelayMS = (1 / 60.0)*1000;
    SDL_AddTimer(out->frameUpdateDelayMS, mod16_window_emit_frame_event, out);
    return out;
};


mod16GraphicsContext_t * mod16_window_get_graphics(mod16Window_t * window) {
    return window->graphics;
}

// Gets the contents of the clipboard as a string.
// If no such contents, the string returned is empty;
matteString_t * mod16_window_get_clipboard(mod16Window_t * window) {
    char * clipboardRaw = SDL_GetClipboardText();
    matteString_t * strVal = matte_string_create_from_c_str("%s", clipboardRaw ? clipboardRaw : "");
    SDL_free(clipboardRaw);
    return strVal;
}

// Sets the clipboard contents to the given string.
void mod16_window_set_clipboard(mod16Window_t * window, const matteString_t * str) {
    SDL_SetClipboardText(matte_string_get_c_str(str));
}

void mod16_window_get_size(mod16Window_t * window, int * w, int * h) {
    SDL_GetWindowSize(window->window, w, h);
}
void mod16_window_set_frame_update_delay(mod16Window_t * window, double delay) {
    window->frameUpdateDelayMS = delay;
}

void mod16_window_set_event_callback(mod16Window_t * window, mod16Window_Event_t evt, mod16_window_event_callback_t func, void * data) {
    window->callbacks[evt] = func;
    window->callbackData[evt] = data;
}


void mod16_window_resolve_events(mod16Window_t * window) {

    SDL_Event evt;
    while(SDL_PollEvent(&evt) != 0) {
        if (evt.type == SDL_QUIT) {
            // TODO: on quit callback via engine 
            exit(0);
            break;
        }            
        
        
        
        switch(evt.type) {
          case SDL_KEYUP: 
          case SDL_KEYDOWN: {          
            if (window->callbacks[MOD16_WINDOW_EVENT__KEY]) {
                mod16Window_Event_Key_t keydata = {};
                keydata.down = evt.type == SDL_KEYDOWN;
                keydata.key = evt.key.keysym.sym;
                
                window->callbacks[MOD16_WINDOW_EVENT__KEY](
                    window,
                    MOD16_WINDOW_EVENT__KEY,
                    &keydata,
                    window->callbackData[MOD16_WINDOW_EVENT__KEY]
                );
            }
            break;
             
          } 
        
          case SDL_TEXTINPUT: {
            if (window->callbacks[MOD16_WINDOW_EVENT__TEXT]) {
                mod16Window_Event_Text_t textdata = {};
                textdata.text = matte_string_create_from_c_str("%s", evt.text.text);

            
                window->callbacks[MOD16_WINDOW_EVENT__TEXT](
                    window,
                    MOD16_WINDOW_EVENT__TEXT,
                    &textdata,
                    window->callbackData[MOD16_WINDOW_EVENT__TEXT]
                );
                
                matte_string_destroy((matteString_t*)textdata.text);
                break;
            }                 
          }
        
        
          case SDL_MOUSEBUTTONDOWN:
          case SDL_MOUSEBUTTONUP: {
          
            if (window->callbacks[MOD16_WINDOW_EVENT__POINTER_BUTTON]) {
                mod16Window_Event_Pointer_t pdata = {};
                pdata.down = evt.button.type == SDL_MOUSEBUTTONDOWN;
                pdata.button = evt.button.button;
                pdata.x = evt.button.x;
                pdata.y = evt.button.y;
                
                window->callbacks[MOD16_WINDOW_EVENT__POINTER_BUTTON](
                    window,
                    MOD16_WINDOW_EVENT__POINTER_BUTTON,
                    &pdata,
                    window->callbackData[MOD16_WINDOW_EVENT__POINTER_BUTTON]
                );
            }
            break;
            
                      
            
          }


          case SDL_MOUSEWHEEL: {
            if (window->callbacks[MOD16_WINDOW_EVENT__POINTER_SCROLL]) {
                mod16Window_Event_Pointer_t pdata = {};
                pdata.x = evt.wheel.x;
                pdata.y = evt.wheel.y;
                
                window->callbacks[MOD16_WINDOW_EVENT__POINTER_SCROLL](
                    window,
                    MOD16_WINDOW_EVENT__POINTER_SCROLL,
                    &pdata,
                    window->callbackData[MOD16_WINDOW_EVENT__POINTER_SCROLL]
                );
            }
            break;
          }

        
          case SDL_MOUSEMOTION: {
            if (window->callbacks[MOD16_WINDOW_EVENT__POINTER_MOTION]) {
                mod16Window_Event_Pointer_t pdata = {};
                pdata.x = evt.motion.x;
                pdata.y = evt.motion.y;
                
                window->callbacks[MOD16_WINDOW_EVENT__POINTER_MOTION](
                    window,
                    MOD16_WINDOW_EVENT__POINTER_MOTION,
                    &pdata,
                    window->callbackData[MOD16_WINDOW_EVENT__POINTER_MOTION]
                );
            }
            break;
          }
            
        }
        
        
        // frame update controlled by timer
        if (evt.type == SDL_USEREVENT) {
            if (window->callbacks[MOD16_WINDOW_EVENT__FRAME_RENDER]) {
                
                window->callbacks[MOD16_WINDOW_EVENT__FRAME_RENDER](
                    window,
                    MOD16_WINDOW_EVENT__FRAME_RENDER,
                    NULL,
                    window->callbackData[MOD16_WINDOW_EVENT__FRAME_RENDER]
                );
            }
            SDL_GL_SwapWindow(window->window);
            
        }


    }

}


void mod16_window_thread_wait(mod16Window_t * w, uint32_t ms) {
    SDL_Delay(ms);
}
double mod16_window_get_ticks(mod16Window_t * w) {
    return SDL_GetTicks();
}





/// internal

SDL_Window * mod16_window_get_sdl_window(mod16Window_t * w) {
    return w->window;
}


