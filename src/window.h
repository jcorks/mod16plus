#ifndef H_SES_WINDOW__INCLUDED
#define H_SES_WINDOW__INCLUDED

#include "./matte/src/matte_string.h"

// sesWindow_t represents a 
// display primitive on the running system and 
// provides additional functionality usually 
// associated with the system such as input
// and timing features.

typedef struct sesWindow_t sesWindow_t;

#include "graphics_context.h"


// Creates a new native system window. 
sesWindow_t * ses_window_create();

// Destroys the resources associated with the window.
void ses_window_destroy(sesWindow_t *);

// Gets the graphics instance associated with the window.
sesGraphicsContext_t * ses_window_get_graphics(sesWindow_t *);

// Gets the contents of the clipboard as a string.
// If no such contents, the string returned is empty;
matteString_t * ses_window_get_clipboard(sesWindow_t *);

// Sets the clipboard contents to the given string.
void ses_window_set_clipboard(sesWindow_t *, const matteString_t *);


// Sets the time per frame in milliseconds 
// Every period, 
void ses_window_set_frame_update_delay(sesWindow_t *, double delay);

// Gets the width / height of the display
void ses_window_get_size(sesWindow_t *, int * w, int * h);

// Gets the number of milliseconds since startup
double ses_window_get_ticks(sesWindow_t *);



typedef enum {
    // The window has processed an event from a 
    // keyboard key press.
    SES_WINDOW_EVENT__KEY,
    
    // The window has processed a text-related event 
    // from the system intended for this display.
    // This can include keyboard input.
    SES_WINDOW_EVENT__TEXT,
    
    // The window has processed a pointer press event.
    SES_WINDOW_EVENT__POINTER_BUTTON,
    
    // The window has processed a scrolling event 
    // relevant to the pointer.
    SES_WINDOW_EVENT__POINTER_SCROLL,
    
    // The window has processed a motion event related 
    // to the pointer.
    SES_WINDOW_EVENT__POINTER_MOTION,
    
    // The window has emitted a frame rendering event 
    // This is regularly emitted.
    SES_WINDOW_EVENT__FRAME_RENDER
} sesWindow_Event_t;


// Data pertaining to a key event.
typedef struct {
    // Whether the key in question is currently pressed down or not.
    int down;
    
    // The key in question
    int key;    
} sesWindow_Event_Key_t;


// Data pertaining to a text event.
typedef struct {
    // a read-only string containing text from 
    // the system for the event. The string is only 
    // valid for the callback call for the event in
    // question.
    const matteString_t * text;    
} sesWindow_Event_Text_t;


// All the targettable buttons for button events.
typedef enum {
    // The left button
    SES_WINDOW_EVENT__POINTER_BUTTON__LEFT,
    
    // The middle button
    SES_WINDOW_EVENT__POINTER_BUTTON__MIDDLE,
    
    // The right button.
    SES_WINDOW_EVENT__POINTER_BUTTON__RIGHT
} sesWindow_Event_PointerButton_t;


// Data pertaining to a pointer event.
typedef struct {
    // whether the mouse button is down
    int down;
        
    // The button pertaining to the event.
    sesWindow_Event_PointerButton_t button;
    
    // The x position of the event in display units.
    float x;
    
    // The y position of the event in display units
    float y;
} sesWindow_Event_Pointer_t;





typedef void (*ses_window_event_callback_t)(sesWindow_t *, sesWindow_Event_t event, void * eventData, void * userData);



// Sets a callback as a response to a sesWindow event.
// The callback is passed the data pertaining to the callback as well 
// as custom data entered when binding the callback to the event.
void ses_window_set_event_callback(sesWindow_t *, sesWindow_Event_t, ses_window_event_callback_t, void * data);


// Scans and applies all pending events, firing 
// any event callbacks for events that are detected.
void ses_window_resolve_events(sesWindow_t *);

// Sleeps on the thread for roughly the given number 
// of milliseconds.
void ses_window_thread_wait(sesWindow_t *, uint32_t ms);

#endif
