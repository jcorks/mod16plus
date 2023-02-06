#ifndef H_MOD16_SDL_GL__INCLUDED
#define H_MOD16_SDL_GL__INCLUDED

#include "../window.h"
#include "../graphics_context.h"

// Abstraction of general OpenGL features
typedef struct mod16SDLGL_t mod16SDLGL_t;


mod16SDLGL_t * mod16_sdl_gl_create(mod16Window_t *);

// Creates a new texture object for sprite objects
int mod16_sdl_gl_new_sprite_texture(mod16SDLGL_t *);

int mod16_sdl_gl_new_bg_texture(mod16SDLGL_t *);

// Sets a tile ID as the "working tile"
// for either writing or reading.
void mod16_sdl_gl_set_sprite_tile(mod16SDLGL_t *, int tileTexture, uint16_t id, const mod16GraphicsContext_Tile_t *);

// Sets a tile ID as the "working tile"
// for either writing or reading.
void mod16_sdl_gl_set_background_tile(mod16SDLGL_t *, int tileTexture, int x, int y, const mod16GraphicsContext_Tile_t *);


// Creates a new mod16 renderable vertex object.
// This stores only mod16GraphicsContext_Vertex_t data
int mod16_sdl_gl_new_vertex_array_set(mod16SDLGL_t *);

// Sets the size of the object. This is NOT to be exceeded 
// when updating the array.
void mod16_sdl_gl_vertex_array_set_size(mod16SDLGL_t *, int object, uint16_t);

// Updates a portion of the array.
void mod16_sdl_gl_vertex_array_update(
    mod16SDLGL_t *,
    int  object,
    uint16_t offset,
    uint16_t count,
    const mod16GraphicsContext_Vertex_t *
);



// Prepares the renderer for a batch of rendering operations.
void mod16_sdl_gl_render_begin(mod16SDLGL_t *);

// Procesmod16 a layer of rendering, ensuring all pending operations get 
// rendered at the same depth level.
void mod16_sdl_gl_render_finish_layer(mod16SDLGL_t *);

void mod16_sdl_gl_render_sprite(
    mod16SDLGL_t *,
    int x, int y,
    float scaleX, float scaleY,
    float centerX, float centerY,
    float rotation,
    int effect,

    mod16Vector_t back,
    mod16Vector_t midBack,
    mod16Vector_t midFront,
    mod16Vector_t front,
            
    int spriteTexture,
    uint32_t id
);

void mod16_sdl_gl_render_background(
    mod16SDLGL_t *,
    int x, int y,

    int effect,

    mod16Vector_t back,
    mod16Vector_t midBack,
    mod16Vector_t midFront,
    mod16Vector_t front,
            
    int backgroundTexture,            
    uint32_t id
);

void mod16_sdl_gl_render_vertices(
    mod16SDLGL_t *,
    const mod16Matrix_t * transform,
    int effect,
    int shape,
    int texture,

    mod16Vector_t back,
    mod16Vector_t midBack,
    mod16Vector_t midFront,
    mod16Vector_t front,

    int vertexArray
);


void mod16_sdl_gl_render_end(mod16SDLGL_t *);



int mod16_sdl_gl_get_render_width(mod16SDLGL_t *);

int mod16_sdl_gl_get_render_height(mod16SDLGL_t *);
 


#endif
