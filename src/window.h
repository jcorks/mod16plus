#ifndef H_MOD16_WINDOW__INCLUDED
#define H_MOD16_WINDOW__INCLUDED

#include "./matte/src/matte_string.h"

// mod16Window_t represents a 
// display primitive on the running system and 
// provides additional functionality usually 
// associated with the system such as input
// and timing features.

typedef struct mod16Window_t mod16Window_t;

#include "graphics_context.h"


// Creates a new native system window. 
mod16Window_t * mod16_window_create();

// Destroys the resources associated with the window.
void mod16_window_destroy(mod16Window_t *);

// Gets the graphics instance associated with the window.
mod16GraphicsContext_t * mod16_window_get_graphics(mod16Window_t *);

// Gets the contents of the clipboard as a string.
// If no such contents, the string returned is empty;
matteString_t * mod16_window_get_clipboard(mod16Window_t *);

// Sets the clipboard contents to the given string.
void mod16_window_set_clipboard(mod16Window_t *, const matteString_t *);


// Sets the time per frame in milliseconds 
// Every period, 
void mod16_window_set_frame_update_delay(mod16Window_t *, double delay);

// Gets the width / height of the display
void mod16_window_get_size(mod16Window_t *, int * w, int * h);

// Gets the number of milliseconds since startup
double mod16_window_get_ticks(mod16Window_t *);



typedef enum {
    // The window has processed an event from a 
    // keyboard key press.
    MOD16_WINDOW_EVENT__KEY,
    
    // The window has processed a text-related event 
    // from the system intended for this display.
    // This can include keyboard input.
    MOD16_WINDOW_EVENT__TEXT,
    
    // The window has processed a pointer press event.
    MOD16_WINDOW_EVENT__POINTER_BUTTON,
    
    // The window has processed a scrolling event 
    // relevant to the pointer.
    MOD16_WINDOW_EVENT__POINTER_SCROLL,
    
    // The window has processed a motion event related 
    // to the pointer.
    MOD16_WINDOW_EVENT__POINTER_MOTION,
    
    // The window has emitted a frame rendering event 
    // This is regularly emitted.
    MOD16_WINDOW_EVENT__FRAME_RENDER
} mod16Window_Event_t;


// Data pertaining to a key event.
typedef struct {
    // Whether the key in question is currently pressed down or not.
    int down;
    
    // The key in question
    int key;    
} mod16Window_Event_Key_t;


// Data pertaining to a text event.
typedef struct {
    // a read-only string containing text from 
    // the system for the event. The string is only 
    // valid for the callback call for the event in
    // question.
    const matteString_t * text;    
} mod16Window_Event_Text_t;


// All the targettable buttons for button events.
typedef enum {
    // The left button
    MOD16_WINDOW_EVENT__POINTER_BUTTON__LEFT,
    
    // The middle button
    MOD16_WINDOW_EVENT__POINTER_BUTTON__MIDDLE,
    
    // The right button.
    MOD16_WINDOW_EVENT__POINTER_BUTTON__RIGHT
} mod16Window_Event_PointerButton_t;


// Data pertaining to a pointer event.
typedef struct {
    // whether the mouse button is down
    int down;
        
    // The button pertaining to the event.
    mod16Window_Event_PointerButton_t button;
    
    // The x position of the event in display units.
    float x;
    
    // The y position of the event in display units
    float y;
} mod16Window_Event_Pointer_t;





typedef void (*mod16_window_event_callback_t)(mod16Window_t *, mod16Window_Event_t event, void * eventData, void * userData);



// Sets a callback as a response to a mod16Window event.
// The callback is passed the data pertaining to the callback as well 
// as custom data entered when binding the callback to the event.
void mod16_window_set_event_callback(mod16Window_t *, mod16Window_Event_t, mod16_window_event_callback_t, void * data);


// Scans and applies all pending events, firing 
// any event callbacks for events that are detected.
void mod16_window_resolve_events(mod16Window_t *);

// Sleeps on the thread for roughly the given number 
// of milliseconds.
void mod16_window_thread_wait(mod16Window_t *, uint32_t ms);

#endif
