#include "../graphics_context.h"
#include "../matte/src/matte_array.h"
#include "gl.h"
#include <string.h>
#include <stdlib.h>



typedef struct {
    // FullSprite_t
    matteArray_t * sprites;
    // FullBackground_t
    matteArray_t * bgs;
    // FullVertexSet_t
    matteArray_t * vertices;

} mod16GraphicsLayer_t;



struct mod16GraphicsContext_t {
    // SDL GLES context
    mod16SDLGL_t * gl;
    
    mod16GraphicsLayer_t layer[MOD16_GRAPHICS_CONTEXT__LAYER_COUNT];

};



typedef struct {
    mod16GraphicsContext_Sprite_t data;
    mod16GraphicsContext_Storage_t * src;
} FullSprite_t;

typedef struct {
    mod16GraphicsContext_Background_t data;
    mod16GraphicsContext_Storage_t * src;
} FullBackground_t;

typedef struct {    
    mod16GraphicsContext_VertexSettings_t data;
    mod16GraphicsContext_Storage_t * src;
} FullVertexSet_t;



struct mod16GraphicsContext_Storage_t {
    mod16GraphicsContext_t * ctx;

    int spriteTexture;
    int bgTexture[MOD16_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_COUNT];

    // the actual object from the SDLGL abstraction
    int vertexArrayObject;
    // whether the object needs its contents re-uploaded
    int vertexArrayOutOfDate;
    // the real size of the array. This is always kept up to date.
    int vertexArraySize;    

    mod16GraphicsContext_Vertex_t vertices[MOD16_GRAPHICS_CONTEXT_STORAGE__VERTEX_COUNT_MAX];
    mod16GraphicsContext_Tile_t tiles[MOD16_GRAPHICS_CONTEXT_STORAGE__TOTAL_TILE_COUNT];
    mod16GraphicsContext_Palette_t palettes[MOD16_GRAPHICS_CONTEXT_STORAGE__PALETTE_COUNT];
   
};



mod16GraphicsContext_t * mod16_graphics_context_create(mod16Window_t * window) {
    mod16GraphicsContext_t * out = calloc(1, sizeof(mod16GraphicsContext_t));
    out->gl = mod16_sdl_gl_create(window);
    int i;
    
    for(i = 0; i < MOD16_GRAPHICS_CONTEXT__LAYER_COUNT; ++i) {
        out->layer[i].sprites = matte_array_create(sizeof(FullSprite_t));
        out->layer[i].bgs = matte_array_create(sizeof(FullBackground_t));
        out->layer[i].vertices = matte_array_create(sizeof(FullVertexSet_t));
    }
    return out;
}

void mod16_graphics_context_get_render_size(mod16GraphicsContext_t * ctx, int * w, int * h) {
    *w = mod16_sdl_gl_get_render_width(ctx->gl);    
    *h = mod16_sdl_gl_get_render_height(ctx->gl);    
}



mod16GraphicsContext_Storage_t * mod16_graphics_context_create_storage(mod16GraphicsContext_t * ctx) {
    mod16GraphicsContext_Storage_t * str = calloc(1, sizeof(mod16GraphicsContext_Storage_t));
    str->vertexArrayObject = -1;
    str->ctx = ctx;
    return str;
}


void mod16_graphics_context_add_sprite(mod16GraphicsContext_t * ctx, mod16GraphicsContext_Sprite_t * spr, mod16GraphicsContext_Storage_t * storage) {
    mod16GraphicsLayer_t * layer = &ctx->layer[spr->layer-(MOD16_GRAPHICS_CONTEXT__LAYER_MIN)];
    FullSprite_t full  = {};
    full.data = *spr;
    full.src = storage;
    matte_array_push(layer->sprites, full); 
}

void mod16_graphics_context_add_background(mod16GraphicsContext_t * ctx, mod16GraphicsContext_Background_t * bg, mod16GraphicsContext_Storage_t * storage) {
    mod16GraphicsLayer_t * layer = &ctx->layer[bg->layer-(MOD16_GRAPHICS_CONTEXT__LAYER_MIN)];
    FullBackground_t full  = {};
    full.data = *bg;
    full.src = storage;
    matte_array_push(layer->bgs, full); 

}

void mod16_graphics_context_add_vertices(mod16GraphicsContext_t * ctx, mod16GraphicsContext_VertexSettings_t * settings, mod16GraphicsContext_Storage_t * storage) {
    mod16GraphicsLayer_t * layer = &ctx->layer[settings->layer-(MOD16_GRAPHICS_CONTEXT__LAYER_MIN)];
    FullVertexSet_t full  = {};
    full.data = *settings;
    full.src = storage;

    if (storage->vertexArrayObject < 0) {
        storage->vertexArrayObject = mod16_sdl_gl_new_vertex_array_set(ctx->gl);
        storage->vertexArrayOutOfDate = 1;
    } 
    if (storage->vertexArrayOutOfDate) {
        // for now just batch and replace whole set.
        mod16_sdl_gl_vertex_array_set_size(ctx->gl, storage->vertexArrayObject, storage->vertexArraySize);
        mod16_sdl_gl_vertex_array_update(
            ctx->gl,
            storage->vertexArrayObject,
            0,
            storage->vertexArraySize,
            storage->vertices
        );
        storage->vertexArrayOutOfDate = 0;
    }
    matte_array_push(layer->vertices, full); 

}


void mod16_graphics_context_render(mod16GraphicsContext_t * ctx) {
    mod16_sdl_gl_render_begin(ctx->gl);
    
    int i, n;
    // draw each layer in order
    for(i = 0; i < MOD16_GRAPHICS_CONTEXT__LAYER_COUNT; ++i) {
        mod16GraphicsLayer_t * layer = ctx->layer+i;
    
        // start with backgrounds
        uint32_t lenBackgrounds = matte_array_get_size(layer->bgs);
        if (lenBackgrounds) { 
            for(n = 0; n < lenBackgrounds; ++n) {        
                FullBackground_t * bg = &matte_array_at(layer->bgs, FullBackground_t, n);            
                if (bg->data.id >= MOD16_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_COUNT) continue;
                const mod16GraphicsContext_Palette_t * p = mod16_graphics_context_storage_get_palette(bg->src, bg->data.palette);
                if (!p) continue;                
                
                mod16_sdl_gl_render_background(
                    ctx->gl,
                    bg->data.x, bg->data.y,
                    bg->data.scaleX, bg->data.scaleY,
                    bg->data.centerX, bg->data.centerY,
                    bg->data.rotation,

                    bg->data.effect,
                    
                    p->back,
                    p->midBack,
                    p->midFront,
                    p->front,
                    
                    bg->src->bgTexture[bg->data.id],
                    bg->data.id
                );

            }
            matte_array_set_size(layer->bgs, 0);

        }
        // then do sprites
        uint32_t len = matte_array_get_size(layer->sprites);
        for(n = 0; n < len; ++n) {        
            FullSprite_t * iter = &matte_array_at(layer->sprites, FullSprite_t, n);
            if (iter->data.tile >= MOD16_GRAPHICS_CONTEXT_STORAGE__SPRITE_TILE_COUNT) continue;
            const mod16GraphicsContext_Palette_t * p = mod16_graphics_context_storage_get_palette(iter->src, iter->data.palette);
            if (!p) continue;                

            
            mod16_sdl_gl_render_sprite(
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
        if (len) {
            mod16_sdl_gl_render_finish_layer(ctx->gl);
            matte_array_set_size(layer->sprites, 0);
        }

        // finally, vertices
        len = matte_array_get_size(layer->vertices);
        for(n = 0; n < len; ++n) {        
            FullVertexSet_t * iter = &matte_array_at(layer->vertices, FullVertexSet_t, n);


            const mod16GraphicsContext_Palette_t * p = mod16_graphics_context_storage_get_palette(iter->src, iter->data.palette);
            if (!p) continue;                

            
            mod16_sdl_gl_render_vertices(
                ctx->gl,
                &iter->data.transform,
                iter->data.effect,
                iter->data.shape,
                iter->data.textured ? iter->src->spriteTexture : -1,                

                p->back,
                p->midBack,
                p->midFront,
                p->front,
                
                iter->src->vertexArrayObject,
                iter->src->vertexArraySize
            );
        }
        if (len) {
            mod16_sdl_gl_render_finish_layer(ctx->gl);
            matte_array_set_size(layer->vertices, 0);
        }

    }

    // commit to framebuffer 0    
    mod16_sdl_gl_render_end(ctx->gl);    
    
}




const mod16GraphicsContext_Tile_t * mod16_graphics_context_storage_get_tile(
    mod16GraphicsContext_Storage_t * storage, 
    uint16_t index
) {
    static mod16GraphicsContext_Tile_t errorT = {};
    if (index >= MOD16_GRAPHICS_CONTEXT_STORAGE__TOTAL_TILE_COUNT) 
        return &errorT;

    return &storage->tiles[index];
}

void mod16_graphics_context_storage_set_tile(mod16GraphicsContext_Storage_t * storage, uint16_t id, const mod16GraphicsContext_Tile_t * data) {
    if (id >= MOD16_GRAPHICS_CONTEXT_STORAGE__TOTAL_TILE_COUNT) 
        return;
        
    storage->tiles[id] = *data;

    if (storage->spriteTexture == 0) {
        storage->spriteTexture = mod16_sdl_gl_new_sprite_texture(storage->ctx->gl);
    }
    
    
    if (id < MOD16_GRAPHICS_CONTEXT_STORAGE__SPRITE_TILE_COUNT) {
        mod16_sdl_gl_set_sprite_tile(
            storage->ctx->gl, 
            storage->spriteTexture, 
            id,
            data    
        );
    } else {
        int backgroundID = (id - MOD16_GRAPHICS_CONTEXT_STORAGE__SPRITE_TILE_COUNT) / (MOD16_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_WIDTH_TILES * MOD16_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_HEIGHT_TILES);
        if (backgroundID >= MOD16_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_COUNT) return;
        int localID = (id - MOD16_GRAPHICS_CONTEXT_STORAGE__SPRITE_TILE_COUNT) % (MOD16_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_WIDTH_TILES * MOD16_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_HEIGHT_TILES);
        int x = localID % MOD16_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_WIDTH_TILES;
        int y = localID / MOD16_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_WIDTH_TILES;
        
        mod16_sdl_gl_set_background_tile(
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
// the mod16GraphicsContext_Storage_t instance.
const mod16GraphicsContext_Palette_t * mod16_graphics_context_storage_get_palette(const mod16GraphicsContext_Storage_t * storage, uint16_t id) {
    static mod16GraphicsContext_Palette_t errorP = {};
    if (id >= MOD16_GRAPHICS_CONTEXT_STORAGE__PALETTE_COUNT) 
        return &errorP;

    return &storage->palettes[id];

}

void mod16_graphics_context_storage_set_palette(mod16GraphicsContext_Storage_t * storage, uint16_t id, const mod16GraphicsContext_Palette_t * p) {
    if (id >= MOD16_GRAPHICS_CONTEXT_STORAGE__PALETTE_COUNT) 
        return;

    storage->palettes[id] = *p;

}


// Gets / sets a vertex.
// Vertices can refer to sprite tiles to be used as textures.
const mod16GraphicsContext_Vertex_t * mod16_graphics_context_storage_get_vertex(const mod16GraphicsContext_Storage_t * storage, uint16_t index) {
    static mod16GraphicsContext_Vertex_t badvtx = {};
    if (index >= storage->vertexArraySize) return &badvtx;

    return storage->vertices+index;
}

void mod16_graphics_context_storage_set_vertex(mod16GraphicsContext_Storage_t * storage, uint16_t index, const mod16GraphicsContext_Vertex_t * data) {
    if (index >= storage->vertexArraySize)
        return;
    storage->vertices[index] = *data;
    storage->vertexArrayOutOfDate = 1;
} 


// Gets / sets the active vertex count. This count corresponds to 
// the physical number of vertices used during drawing
uint16_t mod16_graphics_context_storage_get_vertex_count(const mod16GraphicsContext_Storage_t * storage) {
    return storage->vertexArraySize;
}

void mod16_graphics_context_storage_set_vertex_count(mod16GraphicsContext_Storage_t * storage, uint16_t size) {
    if (size > MOD16_GRAPHICS_CONTEXT_STORAGE__VERTEX_COUNT_MAX)
        return;
    storage->vertexArraySize = size;
    storage->vertexArrayOutOfDate = 1;
}






































