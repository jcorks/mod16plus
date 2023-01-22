#ifndef H_SES_GRAPHICS_CONTEXT__INCLUDED 
#define H_SES_GRAPHICS_CONTEXT__INCLUDED 

#include "linear.h"
#include <stdint.h>

// The graphics context manages all lower-level
// rendering operations within SES. 
// THe implementaton is environment-specific.
typedef struct sesGraphicsContext_t sesGraphicsContext_t;
typedef struct sesGraphicsContext_Storage_t sesGraphicsContext_Storage_t;
typedef struct sesGraphicsContext_Layer_t sesGraphicsContext_Layer_t;

#define SES_GRAPHICS_CONTEXT__LAYER_COUNT      128
#define SES_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS 8
#define SES_GRAPHICS_CONTEXT__LAYER_MIN -63
#define SES_GRAPHICS_CONTEXT__LAYER_MID 0
#define SES_GRAPHICS_CONTEXT__LAYER_MAX 64



typedef struct sesGraphicsContext_Sprite_t sesGraphicsContext_Sprite_t;
struct sesGraphicsContext_Sprite_t {
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
    // The tile is pulled from the used storage state.
    uint16_t tile;
    
    // previous active sprite in linked list
    sesGraphicsContext_Sprite_t * prev;    
    
    // next active sprite in linked list
    sesGraphicsContext_Sprite_t * next;

};






typedef struct {
    // The X position of the background
    float x;
    // the Y position of the background
    float y;
    // The depth layer of the background
    int layer;
    // the rendering effect mode of the background
    int effect;
    // whether the background is enabled.
    int enabled;
    // the ID of the background. This is used to 
    // determine which tiles to use for rendering.
    uint16_t id;


    uint16_t palette;
} sesGraphicsContext_Background_t;



typedef struct {
    // The first color, also called the back color.
    // This is represented with the value 1 in tiles.
    sesVector_t back;

    // The first color, also called the mid-back color.
    // This is represented with the value 2 in tiles.
    sesVector_t midBack;
    
    // The first color, also called the mid-front color.
    // This is represented with the value 3 in tiles.    
    sesVector_t midFront;
    
    // The first color, also called the front color.
    // This is represented with the value 4 in tiles.    
    sesVector_t front; 

} sesGraphicsContext_Palette_t;


typedef struct {
    uint8_t data[SES_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS * SES_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS];
} sesGraphicsContext_Tile_t;

#include "window.h"




// Creates a new graphics context.
sesGraphicsContext_t * ses_graphics_context_create(sesWindow_t *);

// Creates a new storage to be used for the graphics context.
sesGraphicsContext_Storage_t * ses_graphics_context_create_storage(sesGraphicsContext_t *);

// Frees a graphics context.
void ses_graphics_context_destroy(sesGraphicsContext_t *);





// Queues a sprite for rendering using the tile information
// and palette information
void ses_graphics_context_add_sprite(sesGraphicsContext_t *, sesGraphicsContext_Sprite_t *, sesGraphicsContext_Storage_t *);

// Queues a background for rendering using tile and 
// palette information.
void ses_graphics_context_add_background(sesGraphicsContext_t *, sesGraphicsContext_Background_t *, sesGraphicsContext_Storage_t *);

// Renders all queued objects. The queue is then 
// cleared.
void ses_graphics_context_render(sesGraphicsContext_t *);

// Querries the render size of the graphics context.
void ses_graphics_context_get_render_size(sesGraphicsContext_t *, int * w, int * h);




#define SES_GRAPHICS_CONTEXT_STORAGE__SPRITE_TILE_COUNT         1024
#define SES_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_WIDTH_TILES    32
#define SES_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_HEIGHT_TILES   16
#define SES_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_COUNT          32
#define SES_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_TILE_COUNT     (32*(16 * 32))
#define SES_GRAPHICS_CONTEXT_STORAGE__TOTAL_TILE_COUNT          (1024 + (32*(16 * 32))) 
#define SES_GRAPHICS_CONTEXT_STORAGE__PALETTE_COUNT             512


// Reads the contents of a tile. Tiles are from 0 to 
// SES_GRAPHICS_CONTEXT_STORAGE__TOTAL_TILE_COUNT - 1, where the first 
// tiles up to SES_GRAPHICS_CONTEXT_STORAGE__SPRITE_TILE_COUNT tiles are 
// used exclusively for sprites, and the tiles after are used for 
// backgrounds. 
const sesGraphicsContext_Tile_t * ses_graphics_context_storage_get_tile(sesGraphicsContext_Storage_t * storage, uint16_t index);

void ses_graphics_context_storage_set_tile(sesGraphicsContext_Storage_t * storage, uint16_t id, const sesGraphicsContext_Tile_t * data);


// Gets a stored palette These palettes are used when rendering 
// with a sprite or background referring to this palette index within 
// the sesGraphicsContext_Storage_t instance.
const sesGraphicsContext_Palette_t * ses_graphics_context_storage_get_palette(const sesGraphicsContext_Storage_t *, uint16_t id);

void ses_graphics_context_storage_set_palette(sesGraphicsContext_Storage_t *, uint16_t id, const sesGraphicsContext_Palette_t *);












#endif
