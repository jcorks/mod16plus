#ifndef H_SES_CARTRIDGE_INCLUDED
#define H_SES_CARTRIDGE_INCLUDED


typedef struct sesCartridge_t sesCartridge_t;

#include "rom.h"
#include "graphics_context.h"
#include "./matte/src/matte_heap.h"

#define SES_CARTRIDGE__MAX_OSCILLATOR_COUNT     32
#define SES_CARTRIDGE__MAX_WAVEFORM_COUNT       128 
#define SES_CARTRIDGE__MAX_SOURCE_COUNT         64
#define SES_CARTRIDGE__MAX_SUBCARTRIDGE_COUNT   128
#define SES_CARTRIDGE__MAX_SPRITE_COUNT         512 
#define SES_CARTRIDGE__MAX_BACKGROUND_COUNT     32


// Creates a cartridge based on a ROM
sesCartridge_t * ses_cartridge_create(matteVM_t * vm, sesROM_t * rom, sesGraphicsContext_t *, const matteString_t * name, sesCartridge_t * parent);

sesROM_t * ses_cartridge_get_rom(const sesCartridge_t *);


// Gets the context storage for this 
sesGraphicsContext_Storage_t * ses_cartridge_get_context_storage(sesCartridge_t *);


// Initializes and runs the "main" and "output" scripts for 
// Every sub cartridge and this cartridge instance. 
void ses_cartridge_bootup(sesCartridge_t *);

// Gets the singleton instance representing the state of the 
// main script. This is used as the singleton instance given when 
// querying subcartridges under normal operation.
//
// The lifetime of the object persists with the cartridge
// The object is empty until the bootup call is initiated
matteValue_t ses_cartridge_get_main(sesCartridge_t *);


const matteString_t * ses_cartridge_get_name(sesCartridge_t *);

const matteString_t * ses_cartridge_get_fullname(sesCartridge_t *);



// Sets or disables a cartridge sprite as currently active.
// Active sprites are rendered every frame.
// If the index is beyond the max sprite, no action is taken
void ses_cartridge_enable_sprite(sesCartridge_t * cartridge, uint16_t index, int enabled);

// Gets the sprite at the given index. If the index is 
// beyond the max sprite, NULL is returned.
sesGraphicsContext_Sprite_t * ses_cartridge_get_sprite(sesCartridge_t * cartridge, uint16_t index);


// Gets the background at the given index. If the index is 
// beyond the max sprite, NULL is returned.
sesGraphicsContext_Background_t * ses_cartridge_get_background(sesCartridge_t * cartridge, uint16_t index);




typedef struct {
    // locked function from user. It is the user-code's responsibility 
    // to ensure that the function is locked while set to prevent 
    // cleanup from the matte heap.
    matteValue_t function;

    // the length of the alarm wait time in MS
    uint32_t lengthMS;
    
    // when the oscillator was made active
    uint32_t startMS;
    
    uint32_t endMS;

    // whether the oscillator is active
    int active;
    
    // sin for the current 
    float sinValue;

} sesCartridge_Oscillator_t;

// Enables or disables a cartridge oscillator.
// oscillators are repolled with ses_cartridge_poll_oscillators
void ses_cartridge_enable_oscillator(sesCartridge_t *, uint16_t index, int enabled, double ticks);

// Gets the specified oscillator or NULL 
// if there is no such oscillator.
sesCartridge_Oscillator_t *  ses_cartridge_get_oscillator(sesCartridge_t *, uint16_t index);

// Updates oscillators based on a current timestamp.
void ses_cartridge_poll_oscillators(sesCartridge_t *, double ticks);








// Gets all backgrounds and sprites and pushes them to the graphics context
// to then be rendered when ready.
sesCartridge_t * ses_cartridge_push_graphics(sesCartridge_t *, sesGraphicsContext_t *);

// Gets the subcartridge by name. If non exists, NULL is returned.
sesCartridge_t * ses_cartridge_get_subcartridge(sesCartridge_t * rom, const matteString_t *);


// When booting the rom, the active context is the one
// currently booting. This is useful for context controls 
// in the matteVM to discern which cartridge's storage 
// specifics should be referenced at a given time.
sesCartridge_t * ses_cartridge_get_active_boot_context();

// Uniquely identifies the cartridge globally.
uint32_t ses_cartridge_get_id(const sesCartridge_t *);

// Retrieves a matteValue_t representing the cartridge source data (script).
// If the source has not been run yet, it is run. Its return value is the 
// matteValue_t stored. It is cached and returned on subsequent calls to 
// this function.
matteValue_t ses_cartridge_get_source(sesCartridge_t * cart, const matteString_t * source);

// Retrieves the cartridge from 
sesCartridge_t * ses_cartridge_from_id(uint32_t id);


#endif

