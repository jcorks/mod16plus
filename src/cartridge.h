#ifndef H_SES_CARTRIDGE_INCLUDED
#define H_SES_CARTRIDGE_INCLUDED


typedef struct sesCartridge_t sesCartridge_t;



#define SES_CARTRIDGE__MAX_TILE_COUNT           1024
#define SES_CARTRIDGE__MAX_BACKGROUND_COUNT     32
#define SES_CARTRIDGE__MAX_PALETTE_COUNT        512
#define SES_CARTRIDGE__MAX_SPRITE_COUNT         512 
#define SES_CARTRIDGE__MAX_OSCILLATOR_COUNT     32
#define SES_CARTRIDGE__MAX_WAVEFORM_COUNT       128 
#define SES_CARTRIDGE__MAX_SOURCE_COUNT         64
#define SES_CARTRIDGE__MAX_SUBCARTRIDGE_COUNT   128


// Creates a cartridge based on a ROM
sesCartridge_t * ses_cartridge_create(sesROM_t * rom);

sesROM_t * ses_cartridge_get_rom(const sesCartridge_t *);



typedef struct {
    // x position of the sprite
    float x;
    
    // y position of the sprite 
    float y;
    
    // Rotation in degrees of the sprite about the center.
    float rotation;
    
    // scale in the X direction of the sprite about the center
    float scaleX;
    
    // Scale in the Y direction of the sprite about the center
    float scaleY;
    
    // The center X position of the sprite for transform effects.
    float centerX;

    // The center Y position of the sprite for transform effects.
    float centerY;

    // The depth layer to place this sprite
    int layer;
    
    // The effect mode to render this sprite in.
    int effect;
    
    // Whether the sprite is currently enabled.
    int enabled;

    // the palette ID of the sprite
    uint16_t palette;
    
    // The tile representing the sprite.
    uint16_t tile;
    
    // previous active sprite in linked list
    SES_Sprite * prev;    
    
    // next active sprite in linked list
    SES_Sprite * next;

} sesCartridge_Sprite_t;



// Sets or disables a cartridge sprite as currently active.
// Active sprites are rendered every frame.
// If the index is beyond the max sprite, no action is taken
void ses_cartridge_enable_sprite(sesCartridge_t * cartridge, uint16_t index, int enabled);

// Gets the sprite at the given index. If the index is 
// beyond the max sprite, NULL is returned.
sesCartridge_Sprite_t * ses_cartridge_get_sprite(sesCartridge_t * cartridge, uint16_t index);







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
void ses_cartridge_enable_oscillator(sesCartridge_t *, uint16_t index, int enabled);

// Gets the specified oscillator or NULL 
// if there is no such oscillator.
sesCartridge_Oscillator_t *  ses_cartridge_get_oscillator(sesCartridge_t *, uint16_t index);

// Updates oscillators based on a current timestamp.
void ses_cartridge_poll_oscillators(sesCartridge_t *, double ticks);






// Gets all backgrounds and sprites and pushes them to the graphics context
// to then be rendered when ready.
sesCartridge_t * ses_cartridge_push_graphics(sesCartridge_t *, sesGraphicsContext_t *);

// Gets the index'th sub-cartridge loaded by this cartridge.
// Optionally, name may be presented. If non-NULL, the string will be 
// set to the name.
sesCartridge_t * ses_cartridge_get_subcartridge(sesCartridge_t * rom, uint16_t index, matteString_t * name);




#endif

