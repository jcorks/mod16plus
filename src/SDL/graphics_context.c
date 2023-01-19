#include "../graphics_context.h"
#include "gl.h"



typedef struct {
    // FullSprite_t
    matteArray_t * sprites;
    // FullBackground_t
    matteArray_t * bgs;

} sesGraphicsLayer_t;



struct sesGraphicsContext_t {
    // SDL GLES context
    sesSDLGL_t * gl;
    
    sesGraphicsLayer_t layers[SES_GRAPHICS_CONTEXT__LAYER_COUNT]

};



typedef struct {
    sesGraphicsContext_Sprite_t data;
    sesGraphicsContext_t * src;
} FullSprite_;

typedef struct {
    sesGraphicsContext_Background_t data;
    sesGraphicsContext_t * src;
} FullBackground_t;



struct sesGraphicsContext_Storage_t {
    sesGraphicsContext_t * ctx;
    int spriteTexture;
    int bgTexture[SES_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_COUNT];
    
    sesGraphicsContext_Tile_t tiles[SES_GRAPHICS_CONTEXT_STORAGE__TOTAL_TILE_COUNT];
    sesGraphicsContext_Palette_t palettes[SES_GRAPHICS_CONTEXT_STORAGE__PALETTE_COUNT];
}



sesGraphicsContext_t * ses_graphics_context_create(sesWindow_t * window) {
    sesGraphicsContext_t * out = calloc(1, sizeof(sesGraphicsContext_t));
    out->gl = ses_sdl_gl_create(window);
    int i;
    
    for(i = 0; i < SES_GRAPHICS_CONTEXT__LAYER_COUNT; ++i) {
        out->layer[i].sprites = matte_array_create(sizeof(FullSprite_t));
        out->layer[i].bgs = matte_array_create(sizeof(FullBackground_t));
    }
    return out;
}

void ses_graphics_context_get_render_size(sesGraphicsContext_t * ctx, int * w, int * h) {
    *w = ses_sdl_gl_get_render_width(ctx->gl);    
    *h = ses_sdl_gl_get_render_height(ctx->gl);    
}



sesGraphicsContext_Storage_t * ses_graphics_context_create_storage(sesGraphicsContext_t * ctx) {
    sesGraphicsContext_Storage_t * str = calloc(1, sizeof(sesGraphicsContext_Storage_t));
    str->ctx = ctx;
    return str;
}


void ses_graphics_context_layer_add_sprite(sesGraphicsContext_t * ctx, sesGraphicsContext_Sprite_t * spr, sesGraphicsContext_Storage_t * storage) {
    sesGraphicsContext_Layer_t * layer = &ctx->layers[iter->layer-(LAYER_MIN))];
    FullSprite_t full  = {};
    full.data = *spr;
    full.src = storage;
    matte_array_push(layer->sprites, full); 
}

void ses_graphics_context_layer_add_background(sesGraphicsContext_t * ctx, sesGraphicsContext_Background_t * bg, sesGraphicsContext_Storage_t * storage) {
    sesGraphicsContext_Layer_t * layer = &ctx->layers[iter->layer-(LAYER_MIN))];
    FullBackground_t full  = {};
    full.data = *bg;
    full.src = storage;
    matte_array_push(layer->bgs, full); 

}


void ses_graphics_context_render(sesGraphicsContext_t * ctx) {
    ses_sdl_gl_render_begin(ctx->gl);
    

    // draw each layer in order
    for(i = 0; i < SES_GRAPHICS_CONTEXT__LAYER_COUNT; ++i) {
        SES_GraphicsLayer * layer = sdl.main.layers+i;
    
        // start with backgrounds
        uint32_t lenBackgrounds = matte_array_get_size(layer->bgs);
        if (lenBackgrounds) { 
            for(n = 0; n < lenBackgrounds; ++n) {        
                FullBackground_t * bg = &matte_array_at(layer->bgs, FullBackground_t, n);            
                if (bg->data.id >= SES_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_COUNT) continue;
                sesGraphicsContext_Palette_t * p = ses_graphics_context_storage_get_palette(bg->src, bg->palette);
                if (!p) continue;                
                
                ses_sdl_gl_render_background(
                    ctx->gl,
                    bg->data.x, bg->data.y,
                    bg->data.effect,
                    
                    p->back,
                    p->midBack,
                    p->midFront,
                    p->front,
                    
                    bg->src->bgTexture[bg->data.id]
                );

            }
            matte_array_set_size(layer->bgs, 0);

        }
        // then do sprites
        len = matte_array_get_size(layer->sprites);
        if (!len) {
            if (lenBackgrounds) ses_sdl_gl_render_finish_layer(ctx->gl);
            continue;
        }
        for(n = 0; n < len; ++n) {        
            FullSprite_t * iter = matte_array_at(layer->sprites, FullSprite_t *, n);
            if (iter->tile >= SES_GRAPHICS_CONTEXT_STORAGE__SPRITE_TILE_COUNT) continue;
            sesGraphicsContext_Palette_t * p = ses_graphics_context_storage_get_palette(iter->src, iter->palette);
            if (!p) continue;                

            
            ses_sdl_gl_render_sprite(
                ctx->gl,
                iter->data.x, iter->data.y,
                iter->data.scaleX, iter->data.scaleY,
                iter->data.centerX, iter->data.centerY,
                iter->data.rotation,
                iter->data.effect,
                
                p->back,
                p->midBack,
                p->midFront,
                p->front,
                
                iter->src->spriteTexture,
                iter->data.tile
            );
        }
        ses_sdl_gl_render_finish_layer(ctx->gl);
        matte_array_set_size(layer->sprites, 0);
    }

    // commit to framebuffer 0    
    ses_sdl_gl_render_end(ctx->gl);    
    
}




const sesGraphicsContext_Tile_t * ses_graphics_context_storage_get_tile(
    sesGraphicsContext_Storage_t * storage, 
    uint16_t index
) {
    static sesGraphicsContext_Tile_t errorT = {};
    if (index >= SES_GRAPHICS_CONTEXT_STORAGE__TOTAL_TILE_COUNT) 
        return &errorT;

    return &storage->tiles[index];
}

void ses_graphics_context_storage_set_tile(sesGraphicsContext_Storage_t * storage, uint16_t id, const sesGraphicsContext_Tile_t * data) {
    if (index >= SES_GRAPHICS_CONTEXT_STORAGE__TOTAL_TILE_COUNT) 
        return;
        
    storage->tiles[id] = *data;

    if (storage->spriteTexture == 0) {
        storage->spriteTexture = ses_sdl_gl_new_sprite_texture(storage->ctx->gl);
    }
    
    
    if (id < SES_GRAPHICS_CONTEXT_STORAGE__SPRITE_TILE_COUNT) {
        ses_sdl_gl_set_sprite_tile(
            storage->ctx->gl, 
            storage->spriteTexture, 
            id,
            data    
        );
    } else {
        int backgroundID = (id - SES_GRAPHICS_CONTEXT_STORAGE__SPRITE_TILE_COUNT) / (SES_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_WIDTH_TILES * SES_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_HEIGHT_TILES);
        if (backgroundID >= SES_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_COUNT) return;
        int localID = (id - SES_GRAPHICS_CONTEXT_STORAGE__SPRITE_TILE_COUNT) % (SES_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_WIDTH_TILES * SES_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_HEIGHT_TILES)
        int x = localID % SES_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_WIDTH_TILES;
        int y = localID / SES_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_WIDTH_TILES;
        
        ses_sdl_gl_set_background_tile(
            storage->ctx->gl,
            storage->bgTexture[backgroundID],
            x,
            y,
            data
        );
        
    }
        
}


// Gets a stored palette These palettes are used when rendering 
// with a sprite or background referring to this palette index within 
// the sesGraphicsContext_Storage_t instance.
const sesGraphicsContext_Palette_t * ses_graphics_context_storage_get_palette(const sesGraphicsContext_Storage_t * storage, uint16_t id) {
    static sesGraphicsContext_Palette_t errorP = {};
    if (index >= SES_GRAPHICS_CONTEXT_STORAGE__PALETTE_COUNT) 
        return &errorP;

    return &storage->palettes[index];

}

void ses_graphics_context_storage_set_palette(sesGraphicsContext_Storage_t * storage, uint16_t id, const sesGraphicsContext_Palette_t * p) {
    if (index >= SES_GRAPHICS_CONTEXT_STORAGE__PALETTE_COUNT) 
        return;

    storage->palettes[index] = *p;

}





































