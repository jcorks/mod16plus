#ifndef H_SES_SDL_GL__INCLUDED
#define H_SES_SDL_GL__INCLUDED


// Abstraction of general OpenGL features
typedef struct sesSDLGL_t sesSDLGL_t;


sesSDLGL_t * void ses_sdl_gl_create(sesWindow_t *);

// Creates a new texture object for sprite objects
int ses_sdl_gl_new_sprite_texture(sesSDLGL_t *);

int ses_sdl_gl_new_bg_texture(sesSDLGL_t *);

// Sets a tile ID as the "working tile"
// for either writing or reading.
void ses_sdl_gl_set_sprite_tile(sesSDLGL_t *, int tileTexture, uint16_t id, const sesGraphicsContext_Tile_t *);

// Sets a tile ID as the "working tile"
// for either writing or reading.
void ses_sdl_gl_set_background_tile(sesSDLGL_t *, int tileTexture, int x, int y, const sesGraphicsContext_Tile_t *);


// Prepares the renderer for a batch of rendering operations.
void ses_sdl_gl_render_begin(sesSDLGL_t *);

// Processes a layer of rendering, ensuring all pending operations get 
// rendered at the same depth level.
void ses_sdl_gl_render_finish_layer(sesSDLGL_t *);

void ses_sdl_gl_render_sprite(
    sesSDLGL_t *,
    int x, int y,
    float scaleX, float scaleY,
    float centerX, float centerY,
    float rotation,
    int effect,

    sesVector_t back,
    sesVector_t midBack,
    sesVector_t midFront,
    sesVector_t front,
            
    int spriteTexture,
    uint32_t id
);

void ses_sdl_gl_render_background(
    sesSDLGL_t *,
    int x, int y,

    int effect,

    sesVector_t back,
    sesVector_t midBack,
    sesVector_t midFront,
    sesVector_t front,
            
    int backgroundTexture,            
    uint32_t id
);

void ses_sdl_gl_render_end(sesSDLGL_t *);



int ses_sdl_gl_get_render_width(sesSDLGL_t *);

int ses_sdl_gl_get_render_height(sesSDLGL_t *);
 


#endif
