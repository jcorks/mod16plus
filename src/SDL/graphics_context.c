#include "graphics_context.h"



struct sesGraphicsContext_t {
    // SDL GLES context
    SDL_GLContext * ctx;

};






/////// EXTERNAL OPENGL INTERFACE  


// initializes the OpenGL context and window.
extern void ses_sdl_gl_init(SDL_Window ** window, SDL_GLContext ** context);



// Sets a tile ID as the "working tile"
// for either writing or reading.
extern void ses_sdl_gl_bind_tile(uint32_t id);

// Sets a texel / pixel for the currently bound 
// tile ID.
// location should be a number from 0 to 63 (the 
// tile to edit) and value should be the new 
// value to derive the color from the palette, from 0 to 4.
extern void ses_sdl_gl_set_tile_pixel(uint8_t location, uint8_t value);

// Gets the current texel/pixel from the currently bound tile ID.
// location should be a number from 0 to 63
extern uint8_t ses_sdl_gl_get_tile_pixel(uint8_t location);

// Copies tile data from another tile into the bound tile.
extern void ses_sdl_gl_copy_from(uint32_t otherTileID);

// Unbinds a tile, committing a tiles new data if edited.
extern void ses_sdl_gl_unbind_tile();

extern void ses_sdl_gl_render_begin();

extern void ses_sdl_gl_render_finish_layer();

extern void ses_sdl_gl_render_sprite(
    int x, int y,
    float scaleX, float scaleY,
    float centerX, float centerY,
    float rotation,
    int effect,

    sesVector_t back,
    sesVector_t midBack,
    sesVector_t midFront,
    sesVector_t front,
            
    uint32_t id
);

extern void ses_sdl_gl_render_background(
    int x, int y,

    int effect,

    sesVector_t back,
    sesVector_t midBack,
    sesVector_t midFront,
    sesVector_t front,
            
    uint32_t id
);




extern int ses_sdl_gl_get_render_width();

extern int ses_sdl_gl_get_render_height();
 

extern void ses_sdl_gl_render_end();



////////




void ses_graphics_context_render(sesGraphicsContext_t * ctx) {
    ses_sdl_gl_render_begin();
    




    // draw each layer in order
    for(i = 0; i < 128; ++i) {
        SES_GraphicsLayer * layer = sdl.main.layers+i;
    
        // start with backgrounds
        uint32_t lenBackgrounds = matte_array_get_size(layer->bgs);
        if (lenBackgrounds) { 
            for(n = 0; n < lenBackgrounds; ++n) {        
                bg = matte_array_at(layer->bgs, SES_Background *, n);
            
                SES_Palette p = {};
                if (bg->palette < matte_array_get_size(sdl.main.palettes)) {
                    p = matte_array_at(sdl.main.palettes, SES_Palette, bg->palette);
                }
                
                ses_sdl_gl_render_background(
                    bg->x, bg->y,
                    bg->effect,
                    
                    p.back,
                    p.midBack,
                    p.midFront,
                    p.front,
                    
                    bg->id
                );

            }
            matte_array_set_size(layer->bgs, 0);

        }
        // then do sprites
        len = matte_array_get_size(layer->sprites);
        if (!len) {
            if (lenBackgrounds) ses_sdl_gl_render_finish_layer();
            continue;
        }
        for(n = 0; n < len; ++n) {        
            iter = matte_array_at(layer->sprites, SES_Sprite *, n);
        
            SES_Palette p = {};
            if (iter->palette < matte_array_get_size(sdl.main.palettes)) {
                p = matte_array_at(sdl.main.palettes, SES_Palette, iter->palette);
            }

            
            ses_sdl_gl_render_sprite(
                iter->x, iter->y,
                iter->scaleX, iter->scaleY,
                iter->centerX, iter->centerY,
                iter->rotation,
                iter->effect,
                
                p.back,
                p.midBack,
                p.midFront,
                p.front,
                
                iter->tile
            );
        }
        ses_sdl_gl_render_finish_layer();
        matte_array_set_size(layer->sprites, 0);
    }

    // commit to framebuffer 0    
    ses_sdl_gl_render_end();    
    
}

