#ifndef H_MOD16_GRAPHICS_CONTEXT__INCLUDED 
#define H_MOD16_GRAPHICS_CONTEXT__INCLUDED 

#include "linear.h"
#include <stdint.h>

// The graphics context manages all lower-level
// rendering operations within MOD16. 
// THe implementaton is environment-specific.
typedef struct mod16GraphicsContext_t mod16GraphicsContext_t;
typedef struct mod16GraphicsContext_Storage_t mod16GraphicsContext_Storage_t;
typedef struct mod16GraphicsContext_Layer_t mod16GraphicsContext_Layer_t;

#define MOD16_GRAPHICS_CONTEXT__LAYER_COUNT      128
#define MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS 8
#define MOD16_GRAPHICS_CONTEXT__LAYER_MIN -63
#define MOD16_GRAPHICS_CONTEXT__LAYER_MID 0
#define MOD16_GRAPHICS_CONTEXT__LAYER_MAX 64



typedef struct mod16GraphicsContext_Sprite_t mod16GraphicsContext_Sprite_t;
struct mod16GraphicsContext_Sprite_t {
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
    mod16GraphicsContext_Sprite_t * prev;    
    
    // next active sprite in linked list
    mod16GraphicsContext_Sprite_t * next;

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
} mod16GraphicsContext_Background_t;




typedef struct {
    // X position of the vertex
    float x;
    // y position of the vertex
    float y;
    // z position of the vertex
    float z;
    // red color of the vertex. This is interpolated across primitives
    float r;
    // green color of the vertex. This is interpolated across primitives
    float g;
    // blue color of the vertex 
    float b;
    // X coordinate of the tile texture
    float u;
    // Y coordinate of the tile texture
    float v;

    int textured;

    // the tile to use as a texture. 
    uint16_t tile;

    uint16_t palette;
} mod16GraphicsContext_Vertex_t;



typedef struct {
    // The first color, also called the back color.
    // This is represented with the value 1 in tiles.
    mod16Vector_t back;

    // The first color, also called the mid-back color.
    // This is represented with the value 2 in tiles.
    mod16Vector_t midBack;
    
    // The first color, also called the mid-front color.
    // This is represented with the value 3 in tiles.    
    mod16Vector_t midFront;
    
    // The first color, also called the front color.
    // This is represented with the value 4 in tiles.    
    mod16Vector_t front; 

} mod16GraphicsContext_Palette_t;


typedef struct {
    uint8_t data[MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS * MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS];
} mod16GraphicsContext_Tile_t;

#include "window.h"




// Creates a new graphics context.
mod16GraphicsContext_t * mod16_graphics_context_create(mod16Window_t *);

// Creates a new storage to be used for the graphics context.
mod16GraphicsContext_Storage_t * mod16_graphics_context_create_storage(mod16GraphicsContext_t *);

// Frees a graphics context.
void mod16_graphics_context_destroy(mod16GraphicsContext_t *);





// Queues a sprite for rendering using the tile information
// and palette information
void mod16_graphics_context_add_sprite(mod16GraphicsContext_t *, mod16GraphicsContext_Sprite_t *, mod16GraphicsContext_Storage_t *);

// Queues a background for rendering using tile and 
// palette information.
void mod16_graphics_context_add_background(mod16GraphicsContext_t *, mod16GraphicsContext_Background_t *, mod16GraphicsContext_Storage_t *);

// Renders all queued objects. The queue is then 
// cleared.
void mod16_graphics_context_render(mod16GraphicsContext_t *);

// Querries the render size of the graphics context.
void mod16_graphics_context_get_render_size(mod16GraphicsContext_t *, int * w, int * h);




#define MOD16_GRAPHICS_CONTEXT_STORAGE__SPRITE_TILE_COUNT         1024
#define MOD16_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_WIDTH_TILES    32
#define MOD16_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_HEIGHT_TILES   16
#define MOD16_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_COUNT          32
#define MOD16_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_TILE_COUNT     (32*(16 * 32))
#define MOD16_GRAPHICS_CONTEXT_STORAGE__TOTAL_TILE_COUNT          (1024 + (32*(16 * 32))) 
#define MOD16_GRAPHICS_CONTEXT_STORAGE__PALETTE_COUNT             512


// Reads the contents of a tile. Tiles are from 0 to 
// MOD16_GRAPHICS_CONTEXT_STORAGE__TOTAL_TILE_COUNT - 1, where the first 
// tiles up to MOD16_GRAPHICS_CONTEXT_STORAGE__SPRITE_TILE_COUNT tiles are 
// used exclusively for sprites, and the tiles after are used for 
// backgrounds. 
const mod16GraphicsContext_Tile_t * mod16_graphics_context_storage_get_tile(mod16GraphicsContext_Storage_t * storage, uint16_t index);

void mod16_graphics_context_storage_set_tile(mod16GraphicsContext_Storage_t * storage, uint16_t id, const mod16GraphicsContext_Tile_t * data);


// Gets a stored palette These palettes are used when rendering 
// with a sprite or background referring to this palette index within 
// the mod16GraphicsContext_Storage_t instance.
const mod16GraphicsContext_Palette_t * mod16_graphics_context_storage_get_palette(const mod16GraphicsContext_Storage_t *, uint16_t id);

void mod16_graphics_context_storage_set_palette(mod16GraphicsContext_Storage_t *, uint16_t id, const mod16GraphicsContext_Palette_t *);


const mod16GraphicsContext_Vertex_t * mod16_graphics_context_storage_get_vertex(const mod16GraphicsContext_Storage_t *, uint16_t);

void mod16_graphics_context_storage_set_vertex(mod16GraphicsContext_Storage_t *, uint16_t id, const mod16GraphicsContext_Vertex_t *); 


uint16_t mod16_graphics_context_storage_get_vertex_count(const mod16GraphicsContext_Storage_t *);

void mod16_graphics_context_storage_set_vertex_count(const mod16GraphicsContext_Storage_t *);










#endif
