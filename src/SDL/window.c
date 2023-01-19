
#include "../window.h"
#include <SDL2/SDL.h>


struct sesWindow_t {
    SDL_Window    * window;

    sesGraphicsContext_t * graphics;
    
    // delay before updating the next frame
    uint32_t frameUpdateDelayMS;
    
    // callback ID for the timer event emitter
    SDL_TimerID frameUpdateID;
    
    
    // all event callbacks
    ses_window_event_callback_t callbacks[SES_WINDOW_EVENT__FRAME_RENDER+1];

    // corresponding data to the event.
    void * callbackData[SES_WINDOW_EVENT__FRAME_RENDER+1];
    
};

static uint32_t ses_sdl_emit_frame_event(uint32_t interval, void * param) {
    
    SDL_Event event;
    SDL_UserEvent userevent = {};

    userevent.type = SDL_USEREVENT;
    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);    
    return sdl.frameUpdateDelayMS;
}




sesWindow_t * ses_window_create() {
    if (SDL_Init(
        SDL_INIT_TIMER |
        SDL_INIT_VIDEO |
        SDL_INIT_AUDIO |
        SDL_INIT_GAMECONTROLLER            
    ) != 0) {
        printf("SDL2 subsystem init failure.\n");
        exit(1);
    }    
    sdl.frameUpdateDelayMS = (1 / 60.0)*1000;
    
    SDL_AddTimer(sdl.frameUpdateDelayMS, ses_sdl_emit_frame_event, NULL);
    sesWindow_t * out = calloc(1, sizeof(sesWindow_t));
    *window  = SDL_CreateWindow("Sprite Entertainment System", 0, 0, 240*4, 160*4, SDL_WINDOW_OPENGL);
    out->graphics = ses_graphics_context_create(out);
    return out;
};


sesGraphicsContext_t * ses_window_get_graphics(sesWindow_t * window) {
    return window->graphics;
}

// Gets the contents of the clipboard as a string.
// If no such contents, the string returned is empty;
matteString_t * ses_window_get_clipboard(sesWindow_t * window) {
    char * clipboardRaw = SDL_GetClipboardText();
    matteString_t * strVal = matte_string_create_from_c_str("%s", clipboardRaw ? clipboardRaw : "");
    SDL_free(clipboardRaw);
    return strVal;
}

// Sets the clipboard contents to the given string.
void ses_window_set_clipboard(sesWindow_t * window, const matteString_t * str) {
    SDL_SetClipboardText(matte_string_get_c_str(str));
}

void ses_window_get_size(sesWindow_t * window, int * w, int * h) {
    SDL_GetWindowSize(window->window, w, h);
}



void ses_window_resolve_events(sesWindow_t * window) {

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
            if (window->callbacks[SES_WINDOW_EVENT__KEY]) {
                sesWindow_Event_Key_t keydata = {};
                keydata.down = evt.type == SDL_KEYDOWN;
                keydata.key = evt.key.keysym.sym;
                
                window->callbacks[SES_WINDOW_EVENT__KEY](
                    window,
                    SES_WINDOW_EVENT__KEY,
                    &keydata,
                    window->callbackData[SES_WINDOW_EVENT__KEY]
                );
            }
            break;
             
          } 
        
          case SDL_TEXTINPUT: {
            if (window->callbacks[SES_WINDOW_EVENT__TEXT]) {
                sesWindow_Event_Text_t textdata = {};
                textdata.text = matte_string_create_from_c_str("%s", evt.text.text);

            
                window->callbacks[SES_WINDOW_EVENT__TEXT](
                    window,
                    SES_WINDOW_EVENT__TEXT,
                    &textdata,
                    window->callbackData[SES_WINDOW_EVENT__TEXT]
                );
                
                matte_string_destroy(textdata.text);
                break;
            }                 
          }
        
        
          case SDL_MOUSEBUTTONDOWN:
          case SDL_MOUSEBUTTONUP: {
          
            if (window->callbacks[SES_WINDOW_EVENT__POINTER_BUTTON]) {
                sesWindow_Event_Pointer_t pdata = {};
                pdata.down = evt.button.type == SDL_MOUSEBUTTONDOWN;
                pdata.button = evt.button.button;
                pdata.x = evt.button.x;
                pdata.y = evt.button.y;
                
                window->callbacks[SES_WINDOW_EVENT__POINTER_BUTTON](
                    window,
                    SES_WINDOW_EVENT__POINTER_BUTTON,
                    &pointerdata,
                    window->callbackData[SES_WINDOW_EVENT__POINTER_BUTTON]
                );
            }
            break;
            
                      
            
          }


          case SDL_MOUSEWHEEL: {
            if (window->callbacks[SES_WINDOW_EVENT__POINTER_SCROLL]) {
                sesWindow_Event_Pointer_t pdata = {};
                pdata.x = evt.wheel.x;
                pdata.y = evt.wheel.y;
                
                window->callbacks[SES_WINDOW_EVENT__POINTER_SCROLL](
                    window,
                    SES_WINDOW_EVENT__POINTER_SCROLL,
                    &pointerdata,
                    window->callbackData[SES_WINDOW_EVENT__POINTER_SCROLL]
                );
            }
            break;
          }

        
          case SDL_MOUSEMOTION: {
            if (window->callbacks[SES_WINDOW_EVENT__POINTER_MOTION]) {
                sesWindow_Event_Pointer_t pdata = {};
                pdata.x = evt.motion.x;
                pdata.y = evt.motion.y;
                
                window->callbacks[SES_WINDOW_EVENT__POINTER_MOTION](
                    window,
                    SES_WINDOW_EVENT__POINTER_MOTION,
                    &pointerdata,
                    window->callbackData[SES_WINDOW_EVENT__POINTER_MOTION]
                );
            }
            break;
          }
            
        }
        
        
        // frame update controlled by timer
        if (evt.type == SDL_USEREVENT) {
            if (window->callbacks[SES_WINDOW_EVENT__FRAME_RENDER]) {
                
                window->callbacks[SES_WINDOW_EVENT__FRAME_RENDER](
                    window,
                    SES_WINDOW_EVENT__FRAME_RENDER,
                    NULL,
                    window->callbackData[SES_WINDOW_EVENT__FRAME_RENDER]
                );
            }
            SDL_GL_SwapWindow(window->window);
            
        }


    }

}


void ses_window_thread_wait(sesWindow_t * w, uint32_t ms) {
    SDL_Delay(ms);
}
double ses_window_get_ticks(sesWindow_t * w) {
    return SDL_GetTicks();
}


