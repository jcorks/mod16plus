#ifndef H_MOD16_CARTRIDGE_INCLUDED
#define H_MOD16_CARTRIDGE_INCLUDED


typedef struct mod16Cartridge_t mod16Cartridge_t;

#include "rom.h"
#include "graphics_context.h"
#include "./matte/src/matte_heap.h"

#define MOD16_CARTRIDGE__MAX_OSCILLATOR_COUNT     32
#define MOD16_CARTRIDGE__MAX_WAVEFORM_COUNT       128 
#define MOD16_CARTRIDGE__MAX_SOURCE_COUNT         64
#define MOD16_CARTRIDGE__MAX_SUBCARTRIDGE_COUNT   128
#define MOD16_CARTRIDGE__MAX_SPRITE_COUNT         512 
#define MOD16_CARTRIDGE__MAX_BACKGROUND_COUNT     32


// Creates a cartridge based on a ROM
mod16Cartridge_t * mod16_cartridge_create(matteVM_t * vm, mod16ROM_t * rom, mod16GraphicsContext_t *, const matteString_t * name, mod16Cartridge_t * parent);

mod16ROM_t * mod16_cartridge_get_rom(const mod16Cartridge_t *);


// Gets the context storage for this 
mod16GraphicsContext_Storage_t * mod16_cartridge_get_context_storage(mod16Cartridge_t *);


// Initializes and runs the "main" and "output" scripts for 
// Every sub cartridge and this cartridge instance. 
void mod16_cartridge_bootup(mod16Cartridge_t *);

// Gets the singleton instance representing the state of the 
// main script. This is used as the singleton instance given when 
// querying subcartridges under normal operation.
//
// The lifetime of the object persists with the cartridge
// The object is empty until the bootup call is initiated
matteValue_t mod16_cartridge_get_main(mod16Cartridge_t *);


const matteString_t * mod16_cartridge_get_name(mod16Cartridge_t *);

const matteString_t * mod16_cartridge_get_fullname(mod16Cartridge_t *);



// Sets or disables a cartridge sprite as currently active.
// Active sprites are rendered every frame.
// If the index is beyond the max sprite, no action is taken
void mod16_cartridge_enable_sprite(mod16Cartridge_t * cartridge, uint16_t index, int enabled);

// Gets the sprite at the given index. If the index is 
// beyond the max sprite, NULL is returned.
mod16GraphicsContext_Sprite_t * mod16_cartridge_get_sprite(mod16Cartridge_t * cartridge, uint16_t index);


// Gets the background at the given index. If the index is 
// beyond the max sprite, NULL is returned.
mod16GraphicsContext_Background_t * mod16_cartridge_get_background(mod16Cartridge_t * cartridge, uint16_t index);

// Gets the vertex settings for the cartridge.
// This can be edited and will be used when rendering if 
// the vertex count is above 0.
mod16GraphicsContext_VertexSettings_t * mod16_cartridge_get_vertex_settings(mod16Cartridge_t *);





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

} mod16Cartridge_Oscillator_t;

// Enables or disables a cartridge oscillator.
// oscillators are repolled with mod16_cartridge_poll_oscillators
void mod16_cartridge_enable_oscillator(mod16Cartridge_t *, uint16_t index, int enabled, double ticks);

// Gets the specified oscillator or NULL 
// if there is no such oscillator.
mod16Cartridge_Oscillator_t *  mod16_cartridge_get_oscillator(mod16Cartridge_t *, uint16_t index);

// Updates oscillators based on a current timestamp.
void mod16_cartridge_poll_oscillators(mod16Cartridge_t *, double ticks);








// Gets all backgrounds and sprites and pushes them to the graphics context
// to then be rendered when ready.
mod16Cartridge_t * mod16_cartridge_push_graphics(mod16Cartridge_t *, mod16GraphicsContext_t *);

// Gets the subcartridge by name. If non exists, NULL is returned.
mod16Cartridge_t * mod16_cartridge_get_subcartridge(mod16Cartridge_t * rom, const matteString_t *);


// When booting the rom, the active context is the one
// currently booting. This is useful for context controls 
// in the matteVM to discern which cartridge's storage 
// specifics should be referenced at a given time.
mod16Cartridge_t * mod16_cartridge_get_active_boot_context();

// Uniquely identifies the cartridge globally.
uint32_t mod16_cartridge_get_id(const mod16Cartridge_t *);

// Retrieves a matteValue_t representing the cartridge source data (script).
// If the source has not been run yet, it is run. Its return value is the 
// matteValue_t stored. It is cached and returned on subsequent calls to 
// this function.
matteValue_t mod16_cartridge_get_source(mod16Cartridge_t * cart, const matteString_t * source);

// Retrieves the cartridge from 
mod16Cartridge_t * mod16_cartridge_from_id(uint32_t id);


#endif

