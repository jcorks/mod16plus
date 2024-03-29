#include "../../matte/src/matte_array.h"
#include "../../matte/src/matte_table.h"
#include "../../linear.h"
#include "../gl.h"
#include "glad/include/glad/glad.h"
#include <SDL2/SDL.h>


#define SPRITE_TEX_LENGTH_TILES 32


typedef struct {
    GLuint handle;
    GLuint depthRB;
    GLuint texture;
    int width;
    int height;
} MOD16_GLFramebuffer;


typedef enum {
    RE_COLOR,
    RE_MASK,
    RE_MASK_AND_COLOR,
    RE_COLOR_ON_MASK,
    RE_COLOR_AROUND_MASK,
    RE_BLEND,
    RE_BLEND_ON_MASK,
    RE_BLEND_AROUND_MASK,
    RE_COUNT
} RenderEffect;



typedef struct {
    GLuint handle;
    GLuint vertexShader;
    GLuint fragmentShader;
    
    GLint locationVBOposition;
    GLint locationVBOuv;
    GLint locationVBOback;
    GLint locationVBOmidBack;
    GLint locationVBOmidFront;
    GLint locationVBOfront;
    GLint locationVBOtop;
    GLint locationVBObase;
    
    GLint locationUniformProj;
    GLint locationUniformEffect;
    GLint locationUniformTexture;
    GLint locationUniformUseStaticColors;

    GLint locationUniformBackStatic;
    GLint locationUniformMidBackStatic;
    GLint locationUniformMidFrontStatic;
    GLint locationUniformFrontStatic;   
    GLint locationUniformTopStatic;   



    GLint vbo;
} MOD16_GLProgram;

typedef struct {
    float x;
    float y;
    float z;

    float u;
    float v;

    float colorBackR;
    float colorBackG;
    float colorBackB;

    float colorMidBackR;
    float colorMidBackG;
    float colorMidBackB;

    float colorMidFrontR;
    float colorMidFrontG;
    float colorMidFrontB;

    float colorFrontR;
    float colorFrontG;
    float colorFrontB;

    float colorTopR;
    float colorTopG;
    float colorTopB;


    float colorBaseR;
    float colorBaseG;
    float colorBaseB;

} MOD16_VBOvertex;



typedef struct {
    GLint texture;
    int effect;
    matteArray_t * vertices;
} MOD16_GLSpriteBatch;


typedef struct {
    int effect;
    int texture;
    
    int x0; int y0;
    int x1; int y1;
    int x2; int y2;
    int x3; int y3;

    mod16Vector_t back;
    mod16Vector_t midBack;
    mod16Vector_t midFront;
    mod16Vector_t front;
    mod16Vector_t top;
    
    uint32_t id;

} MOD16_GLBackgroundBatch;


typedef struct {
    mod16Matrix_t transform;
    int effect;
    int shape;
    int texture;
    mod16Vector_t back;
    mod16Vector_t midBack;
    mod16Vector_t midFront;
    mod16Vector_t front;
    mod16Vector_t top;
    GLint vbo;
    int count;
} MOD16_GLVertexBatch;

struct mod16SDLGL_t {

    
    // The raw framebuffer that holds the low-resolution
    // results. This will be of the user's requested size
    MOD16_GLFramebuffer resultFramebuffer;
    MOD16_GLProgram spriteProgram;
    MOD16_GLProgram screenProgram;
    
    int resolutionWidth;
    int resolutionHeight;

    matteTable_t * spriteBatches;
    matteArray_t * spriteBatchPool;
    matteTableIter_t * spriteBatchIter;
    matteArray_t * bgBatches;
    matteArray_t * vertexBatches;
    mod16Window_t * srcWindow;
    SDL_GLContext context;


    mod16Matrix_t proj;
};



static MOD16_GLProgram create_program(const char * srcVertex, const char * srcFragment);
static MOD16_GLFramebuffer create_framebuffer(int w, int h);
static int mod16_sdl_gl_get_error();
static void mod16_sdl_gl_get_tile_attribs(uint32_t id, float * u, float * v, float * u1, float * v1);
static void mod16_sdl_gl_2d_transform(
    float x, float y,
    float width, float height,
    float centerX, float centerY,
    float scaleX, float scaleY,
    float rotation,
    
    int * x0, int * y0,
    int * x1, int * y1,
    int * x2, int * y2,
    int * x3, int * y3
);
static uint8_t background_empty[
    MOD16_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_WIDTH_TILES *
    MOD16_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_HEIGHT_TILES *
    MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS *
    MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS
] = {};
static void projection_orthographic(mod16Matrix_t * out,
    float left, float right,
    float top, float bottom,
    float zNear, float zFar
);
static void mod16_sdl_gl_render_background_batch(mod16SDLGL_t * gl, MOD16_GLBackgroundBatch * batch);
static void mod16_sdl_gl_render_sprite_batch(mod16SDLGL_t * gl, MOD16_GLSpriteBatch * batch);
static void mod16_sdl_gl_render_vertex_batch(mod16SDLGL_t * gl, MOD16_GLVertexBatch * batch);


#include "shaders/mod16_sdl__renderer_gl31__sprite_frag_data"
#include "shaders/mod16_sdl__renderer_gl31__sprite_vtx_data"
#include "shaders/mod16_sdl__renderer_gl31__screen_frag_data"
#include "shaders/mod16_sdl__renderer_gl31__screen_vtx_data"




// Internal function to expose SDL features.
extern SDL_Window * mod16_window_get_sdl_window(mod16Window_t *);

mod16SDLGL_t * mod16_sdl_gl_create(mod16Window_t * w) {
    mod16SDLGL_t * gl = calloc(1, sizeof(mod16SDLGL_t));    

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);        
    gl->srcWindow = w;    
    gl->context = SDL_GL_CreateContext(mod16_window_get_sdl_window(w));

    gladLoadGLLoader(SDL_GL_GetProcAddress);

    // use a single VAO for now to match the GLES2 implementation
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    
    // default is gba resolution    
    gl->resultFramebuffer = create_framebuffer(MOD16_GRAPHICS_CONTEXT__RENDER_WIDTH, MOD16_GRAPHICS_CONTEXT__RENDER_HEIGHT);
    gl->resolutionWidth = MOD16_GRAPHICS_CONTEXT__RENDER_WIDTH;
    gl->resolutionHeight = MOD16_GRAPHICS_CONTEXT__RENDER_HEIGHT;

    gl->spriteProgram = create_program(
        mod16_sdl__renderer_gl31__sprite_vtx_data,
        mod16_sdl__renderer_gl31__sprite_frag_data
    );
    
    gl->screenProgram = create_program(
        mod16_sdl__renderer_gl31__screen_vtx_data,
        mod16_sdl__renderer_gl31__screen_frag_data
    );
    
    SDL_GL_SetSwapInterval(0);
    gl->bgBatches = matte_array_create(sizeof(MOD16_GLBackgroundBatch));
    gl->vertexBatches = matte_array_create(sizeof(MOD16_GLVertexBatch));
    gl->spriteBatchPool = matte_array_create(sizeof(MOD16_GLSpriteBatch *));
    gl->spriteBatches = matte_table_create_hash_pointer();
    gl->spriteBatchIter = matte_table_iter_create();
    return gl;
}



int mod16_sdl_gl_new_sprite_texture(mod16SDLGL_t * gl) {
    GLint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    uint8_t * emptyTexture = calloc(1, MOD16_GRAPHICS_CONTEXT_STORAGE__SPRITE_TILE_COUNT * MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS * MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS);
    int tileLength = SPRITE_TEX_LENGTH_TILES;

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_R8,
        tileLength * MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS,
        tileLength * MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        emptyTexture
    );
    free(emptyTexture);

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_NEAREST
    );


    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_NEAREST
    );
    
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}


int mod16_sd_gl_new_background_texture(mod16SDLGL_t * gl) {
    GLint bg = 0;
    glGenTextures(1, &bg);
    glBindTexture(GL_TEXTURE_2D, bg);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_R8,
        MOD16_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_WIDTH_TILES * MOD16_GRAPHICS_CONTEXT_STORAGE__SPRITE_TILE_COUNT,
        MOD16_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_HEIGHT_TILES * MOD16_GRAPHICS_CONTEXT_STORAGE__SPRITE_TILE_COUNT,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        background_empty
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_NEAREST
    );


    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_NEAREST
    );
    
    glBindTexture(GL_TEXTURE_2D, 0);
    return bg;
}


void mod16_sdl_gl_set_sprite_tile(mod16SDLGL_t * gl, int tileTexture, uint16_t index, const mod16GraphicsContext_Tile_t * data) {
    mod16GraphicsContext_Tile_t converted = {};
    int i;
    for(i = 0; i < 64; ++i) {
        converted.data[i] = data->data[i] * (255 / 5.0);
    }

    glBindTexture(GL_TEXTURE_2D, tileTexture);
    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        (index % SPRITE_TEX_LENGTH_TILES)*MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS,
        (index / SPRITE_TEX_LENGTH_TILES)*MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS,
        MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS,
        MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS,
        GL_RED,
        GL_UNSIGNED_BYTE,
        converted.data // always 64 bytes
    );
    glBindTexture(GL_TEXTURE_2D, 0);  
}

void mod16_sdl_gl_set_background_tile(mod16SDLGL_t * gl, int bgTexture, int x, int y, const mod16GraphicsContext_Tile_t * data) {
    glBindTexture(GL_TEXTURE_2D, bgTexture);
    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        (x)*MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS,
        (y)*MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS,
        MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS,
        MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS,
        GL_RED,
        GL_UNSIGNED_BYTE,
        data->data // always 64 bytes
    );
    glBindTexture(GL_TEXTURE_2D, 0);     
    
}


void mod16_sdl_gl_render_begin(mod16SDLGL_t * gl) {
    glViewport(0, 0, MOD16_GRAPHICS_CONTEXT__RENDER_WIDTH, MOD16_GRAPHICS_CONTEXT__RENDER_HEIGHT);

    glBindFramebuffer(GL_FRAMEBUFFER, gl->resultFramebuffer.handle);
    glClearColor(0.12, 0.051, 0.145, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



    glUseProgram(gl->spriteProgram.handle);

    projection_orthographic(
        &gl->proj, 
        0, gl->resolutionWidth,
        0, gl->resolutionHeight,
       -100, 100
    );

    glUniformMatrix4fv(
        gl->spriteProgram.locationUniformProj,
        1, // count,
        1, // transpose (yes)
        (GLfloat*)&gl->proj
    );
    
    
    glEnableVertexAttribArray(gl->spriteProgram.locationVBOposition);
    glEnableVertexAttribArray(gl->spriteProgram.locationVBOuv);
    glEnableVertexAttribArray(gl->spriteProgram.locationVBOback);
    glEnableVertexAttribArray(gl->spriteProgram.locationVBOmidBack);
    glEnableVertexAttribArray(gl->spriteProgram.locationVBOmidFront);
    glEnableVertexAttribArray(gl->spriteProgram.locationVBOfront);
    glEnableVertexAttribArray(gl->spriteProgram.locationVBOtop);
    glEnableVertexAttribArray(gl->spriteProgram.locationVBObase);
    
    
    
}




void mod16_sdl_gl_render_finish_layer(mod16SDLGL_t * gl) {
    glEnable(GL_DEPTH_TEST);
    // actually render everything
    // start with the backgrounds 
    int i, n;
    for(i = 0; i < matte_array_get_size(gl->bgBatches); ++i) {
        MOD16_GLBackgroundBatch * batch = &matte_array_at(gl->bgBatches, MOD16_GLBackgroundBatch, i);
        mod16_sdl_gl_render_background_batch(gl, batch);
    }
    matte_array_set_size(gl->bgBatches, 0);


    // then sprites
   
    for(
        matte_table_iter_start(gl->spriteBatchIter, gl->spriteBatches);
        !matte_table_iter_is_end(gl->spriteBatchIter);
        matte_table_iter_proceed(gl->spriteBatchIter)
    ) {
        MOD16_GLSpriteBatch * batch = matte_table_iter_get_value(gl->spriteBatchIter);
        mod16_sdl_gl_render_sprite_batch(gl, batch);
        matte_array_set_size(batch->vertices, 0);
        matte_array_push(gl->spriteBatchPool, batch);
    }
    matte_table_clear(gl->spriteBatches);

    // finally vertices
    for(i = 0; i < matte_array_get_size(gl->vertexBatches); ++i) {
        MOD16_GLVertexBatch * batch = &matte_array_at(gl->vertexBatches, MOD16_GLVertexBatch, i);
        mod16_sdl_gl_render_vertex_batch(gl, batch);
    }
    matte_array_set_size(gl->vertexBatches, 0);
    glFinish();
}

static void mod16_sdl_gl_2d_transform(
    float x, float y,
    float width, float height,
    
    float centerX, float centerY,
    float scaleX, float scaleY,
    float rotation,
    
    int * x0real, int * y0real,
    int * x1real, int * y1real,
    int * x2real, int * y2real,
    int * x3real, int * y3real
) {
    *x0real = centerX * scaleX;
    *y0real = centerY * scaleY;
    *x2real = (centerX + width) * scaleX;
    *y2real = (centerY + height) * scaleY;

    *x1real = *x2real;
    *y1real = *y0real;
    *x3real = *x0real;
    *y3real = *y2real;


    if (rotation) {
        float x0realn = *x0real * cos(rotation / 180.0 * M_PI) - *y0real * sin(rotation / 180.0 * M_PI);
        float y0realn = *y0real * cos(rotation / 180.0 * M_PI) + *x0real * sin(rotation / 180.0 * M_PI);
        float x1realn = *x1real * cos(rotation / 180.0 * M_PI) - *y1real * sin(rotation / 180.0 * M_PI);
        float y1realn = *y1real * cos(rotation / 180.0 * M_PI) + *x1real * sin(rotation / 180.0 * M_PI);
        float x2realn = *x2real * cos(rotation / 180.0 * M_PI) - *y2real * sin(rotation / 180.0 * M_PI);
        float y2realn = *y2real * cos(rotation / 180.0 * M_PI) + *x2real * sin(rotation / 180.0 * M_PI);
        float x3realn = *x3real * cos(rotation / 180.0 * M_PI) - *y3real * sin(rotation / 180.0 * M_PI);
        float y3realn = *y3real * cos(rotation / 180.0 * M_PI) + *x3real * sin(rotation / 180.0 * M_PI);

        *x0real = x0realn;
        *y0real = y0realn;
        *x1real = x1realn;
        *y1real = y1realn;
        *x2real = x2realn;
        *y2real = y2realn;
        *x3real = x3realn;
        *y3real = y3realn;
    }
    *x0real += x;
    *y0real += y;
    *x1real += x;
    *y1real += y;
    *x2real += x;
    *y2real += y;
    *x3real += x;
    *y3real += y;
}



#define SPRITE_BATCH_KEY(__E__, __T__) ((__E__) + (__T__)*10)

void mod16_sdl_gl_render_sprite(
    mod16SDLGL_t * gl,
    int x, int y,
    float scaleX, float scaleY,
    float centerX, float centerY,
    float rotation,
    int effect,

    mod16Vector_t back,
    mod16Vector_t midBack,
    mod16Vector_t midFront,
    mod16Vector_t front,
    mod16Vector_t top,

    int spriteTexture,
    uint32_t id
) {

    

    // order:
    /*
        - center translation
        - scale
        - rotation 
        - position
    */
    int x0real, y0real,
        x1real, y1real,
        x2real, y2real,
        x3real, y3real;
    mod16_sdl_gl_2d_transform(
        x, y,
        MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS, MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS,
        centerX, centerY,
        scaleX, scaleY,
        rotation,
        
        &x0real, &y0real,
        &x1real, &y1real,
        &x2real, &y2real,
        &x3real, &y3real
    );




    float u0, v0, u1, v1;        
    mod16_sdl_gl_get_tile_attribs(id, &u0, &v0, &u1, &v1);
    MOD16_VBOvertex base = {x0real, y0real, 0, u0, v0,    back.x, back.y, back.z,     midBack.x, midBack.y, midBack.z,    midFront.x, midFront.y, midFront.z,    front.x, front.y, front.z, top.x, top.y, top.z,1.f, 1.f, 1.f };    
    MOD16_VBOvertex vboData[6] = {
        base,
        base,
        base,
        base,
        base,
        base
    };
    
    vboData[1].x = x1real;
    vboData[1].y = y1real;
    vboData[1].u = u1;
    vboData[1].v = v0;
    

    vboData[2].x = x2real;
    vboData[2].y = y2real;
    vboData[2].u = u1;
    vboData[2].v = v1;


    vboData[3].x = x2real;
    vboData[3].y = y2real;
    vboData[3].u = u1;
    vboData[3].v = v1;

    vboData[4].x = x3real;
    vboData[4].y = y3real;
    vboData[4].u = u0;
    vboData[4].v = v1;



    MOD16_GLSpriteBatch * batch = matte_table_find_by_int(gl->spriteBatches, SPRITE_BATCH_KEY(effect, spriteTexture));
    if (batch == NULL) {
        if (!matte_array_get_size(gl->spriteBatchPool)) {
            batch = calloc(1, sizeof(MOD16_GLSpriteBatch));
            batch->vertices = matte_array_create(sizeof(MOD16_VBOvertex));
        } else {
            batch = matte_array_at(gl->spriteBatchPool, MOD16_GLSpriteBatch *, matte_array_get_size(gl->spriteBatchPool)-1);
            matte_array_set_size(gl->spriteBatchPool, matte_array_get_size(gl->spriteBatchPool)-1);
        }
        batch->effect = effect;
        batch->texture = spriteTexture;
        matte_table_insert_by_int(gl->spriteBatches, SPRITE_BATCH_KEY(effect, spriteTexture), batch);
    }
    matte_array_push_n(batch->vertices, vboData, 6);
}



void mod16_sdl_gl_render_background(
    mod16SDLGL_t * gl,
    int x, int y,
    float scaleX, float scaleY,
    float centerX, float centerY,
    float rotation,
    int effect,

    mod16Vector_t back,
    mod16Vector_t midBack,
    mod16Vector_t midFront,
    mod16Vector_t front,
    mod16Vector_t top,
    
    int backgroundTexture,
    uint32_t id
) {

    MOD16_GLBackgroundBatch b = {};
    b.back = back;
    b.midBack = midBack;
    b.midFront = midFront;
    b.front = front;
    b.top = top;
    b.id = id;
    b.texture = backgroundTexture;

    mod16_sdl_gl_2d_transform(
        x, y,
        MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS * MOD16_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_WIDTH_TILES,
        MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS * MOD16_GRAPHICS_CONTEXT_STORAGE__BACKGROUND_HEIGHT_TILES,
        centerX, centerY,
        scaleX, scaleY,
        rotation,
        
        &b.x0, &b.y0,
        &b.x1, &b.y1,
        &b.x2, &b.y2,
        &b.x3, &b.y3
    );
    
    
    matte_array_push(gl->bgBatches, b);
}

void mod16_sdl_gl_render_vertices(
    mod16SDLGL_t * gl,
    const mod16Matrix_t * transform,
    int effect,
    int shape,
    int texture,

    mod16Vector_t back,
    mod16Vector_t midBack,
    mod16Vector_t midFront,
    mod16Vector_t front,
    mod16Vector_t top,

    int vertexArray,
    int count
) {
    MOD16_GLVertexBatch v = {};
    v.vbo = vertexArray;
    v.transform = *transform;
    v.effect = effect;
    v.shape = shape;
    v.back = back;
    v.texture = texture;
    v.midBack = midBack;
    v.midFront = midFront;
    v.front = front;
    v.top = top;
    v.count = count;
    matte_array_push(gl->vertexBatches, v);
}




// cleans up + draws to main framebuffer
void mod16_sdl_gl_render_end(mod16SDLGL_t * gl) {
    glDisable(GL_BLEND);
    glDisableVertexAttribArray(gl->spriteProgram.locationVBOposition);
    glDisableVertexAttribArray(gl->spriteProgram.locationVBOuv);
    glDisableVertexAttribArray(gl->spriteProgram.locationVBOback);
    glDisableVertexAttribArray(gl->spriteProgram.locationVBOmidBack);
    glDisableVertexAttribArray(gl->spriteProgram.locationVBOmidFront);
    glDisableVertexAttribArray(gl->spriteProgram.locationVBOfront);
    glDisableVertexAttribArray(gl->spriteProgram.locationVBOtop);
    glDisableVertexAttribArray(gl->spriteProgram.locationVBObase);

    glUseProgram(gl->screenProgram.handle);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    int w, h;
    mod16_window_get_size(gl->srcWindow, &w, &h);
    glViewport(0, 0, w, h);
    
    glClearColor(1, 0, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    MOD16_VBOvertex vboData[] = {
        {-1, -1, 0, 0},
        {1, -1, 1, 0},
        {1, 1, 1, 1},

        {1, 1, 1, 1},
        {-1, 1, 0, 1},
        {-1, -1, 0, 0}
    };    

    glEnableVertexAttribArray(gl->screenProgram.locationVBOposition);
    glEnableVertexAttribArray(gl->screenProgram.locationVBOuv);
    

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gl->resultFramebuffer.texture);



    glBindBuffer(GL_ARRAY_BUFFER, gl->screenProgram.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(MOD16_VBOvertex)*6, vboData, GL_DYNAMIC_DRAW);
    
    glVertexAttribPointer(gl->screenProgram.locationVBOposition, 2, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)0);
    glVertexAttribPointer(gl->screenProgram.locationVBOuv,       2, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)(sizeof(float)*2));

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);


    glDisableVertexAttribArray(gl->screenProgram.locationVBOposition);
    glDisableVertexAttribArray(gl->screenProgram.locationVBOuv);
}


int mod16_sdl_gl_get_render_width(mod16SDLGL_t * gl) {
    return gl->resolutionWidth;
}

int mod16_sdl_gl_get_render_height(mod16SDLGL_t * gl) {
    return gl->resolutionHeight;
}


















static MOD16_GLProgram create_program(const char * srcVertex, const char * srcFragment) {
    MOD16_GLProgram program = {};
    program.handle = glCreateProgram();
    program.vertexShader = glCreateShader(GL_VERTEX_SHADER);
    program.fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    
    glShaderSource(
        program.vertexShader,
        1,
        &srcVertex,
        NULL
    );
    glCompileShader(program.vertexShader);
    GLint result;
    glGetShaderiv(program.vertexShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        int logLen = 2048;
        char * log = malloc(logLen);
        glGetShaderInfoLog(
            program.vertexShader,
            logLen,
            NULL,
            log
        );
        printf("GL31: Vertex shader failed to compile. Log:\n%s\n", log);
        free(log);
        exit(10);    
    }
    glAttachShader(program.handle, program.vertexShader);
    


    glShaderSource(
        program.fragmentShader,
        1,
        &srcFragment,
        NULL
    );
    glCompileShader(program.fragmentShader);
    glGetShaderiv(program.fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        int logLen = 2048;
        char * log = malloc(logLen);
        glGetShaderInfoLog(
            program.fragmentShader,
            logLen,
            NULL,
            log
        );
        printf("GL31: Fragment shader failed to compile. Log:\n%s\n", log);
        free(log);
        exit(10);    
    }
    glAttachShader(program.handle, program.fragmentShader);
    
    
    glLinkProgram(program.handle);
    glGetProgramiv(program.handle, GL_LINK_STATUS, &result);
    if (!result) {
        int logLen = 2048;
        char * log = malloc(logLen);
        glGetProgramInfoLog(
            program.handle,
            logLen,
            NULL,
            log
        );
        printf("GL31: Program failed to link. Log:\n%s\n", log);
        free(log);
        exit(112);

    }

    glDeleteShader(program.vertexShader);
    glDeleteShader(program.fragmentShader);
    
    
    program.locationVBOposition = glGetAttribLocation(program.handle, "position");
    program.locationVBOuv = glGetAttribLocation(program.handle, "uv");
    program.locationVBOback = glGetAttribLocation(program.handle, "colorBack");
    program.locationVBOmidBack = glGetAttribLocation(program.handle, "colorMidBack");
    program.locationVBOmidFront = glGetAttribLocation(program.handle, "colorMidFront");
    program.locationVBOfront = glGetAttribLocation(program.handle, "colorFront");
    program.locationVBOtop = glGetAttribLocation(program.handle, "colorTop");
    program.locationVBObase = glGetAttribLocation(program.handle, "colorBase");

    program.locationUniformEffect = glGetUniformLocation(program.handle, "effect");
    program.locationUniformProj = glGetUniformLocation(program.handle, "proj");
    program.locationUniformTexture = glGetUniformLocation(program.handle, "texture");
    program.locationUniformUseStaticColors = glGetUniformLocation(program.handle, "useStaticColors");
    program.locationUniformBackStatic = glGetUniformLocation(program.handle, "backStatic");
    program.locationUniformMidBackStatic = glGetUniformLocation(program.handle, "midBackStatic");
    program.locationUniformMidFrontStatic = glGetUniformLocation(program.handle, "midFrontStatic");
    program.locationUniformFrontStatic = glGetUniformLocation(program.handle, "frontStatic");
    program.locationUniformTopStatic = glGetUniformLocation(program.handle, "topStatic");
    
    glGenBuffers(1, &program.vbo);
    
    return program;
    
}





int mod16_sdl_gl_new_vertex_array_set(mod16SDLGL_t * gl) {
    GLint out = 0;
    glGenBuffers(1, &out);
    return out;
}

// Sets the size of the object. This is NOT to be exceeded 
// when updating the array.
void mod16_sdl_gl_vertex_array_set_size(mod16SDLGL_t * gl, int object, uint16_t count) {
    glBindBuffer(GL_ARRAY_BUFFER, object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(MOD16_VBOvertex)*count, 0, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Updates a portion of the array.
void mod16_sdl_gl_vertex_array_update(
    mod16SDLGL_t * gl,
    int object,
    uint16_t offset,
    uint16_t count,
    const mod16GraphicsContext_Vertex_t * vbo
) {
    glBindBuffer(GL_ARRAY_BUFFER, object);

    MOD16_VBOvertex * convertex = malloc(sizeof(MOD16_VBOvertex)*count);
    int i;
    for(i = 0; i < count; ++i) {
        float u0, v0, u1, v1;        
        mod16_sdl_gl_get_tile_attribs(vbo[i].tile, &u0, &v0, &u1, &v1);

        convertex[i].u = u0 + vbo[i].u * u1;
        convertex[i].v = v0 + vbo[i].v * v1;


        convertex[i].x = vbo[i].x;
        convertex[i].y = vbo[i].y;
        convertex[i].z = vbo[i].z;


        convertex[i].colorBaseR = vbo[i].r;
        convertex[i].colorBaseG = vbo[i].g;
        convertex[i].colorBaseB = vbo[i].b;
    };
    

    glBufferSubData(
        GL_ARRAY_BUFFER, 
        offset * sizeof(MOD16_VBOvertex), 
        count  * sizeof(MOD16_VBOvertex),
        convertex
    );
    free(convertex);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

}




















static void mod16_sdl_gl_get_tile_attribs(uint32_t id, float * u, float * v, float * u1, float * v1) {
    *u = 0;
    *v = 0;
    int index = id;            
    int TILES_PER_ROW = SPRITE_TEX_LENGTH_TILES;
    int REAL_TEX_SIZE = TILES_PER_ROW * MOD16_GRAPHICS_CONTEXT__TILE_SIZE_PIXELS;
        
    // sub-pixel correction to prevent bleed
    *u = (index % TILES_PER_ROW) / (float)TILES_PER_ROW + (1 / (float)REAL_TEX_SIZE)*0.001;
    *v = (index / TILES_PER_ROW) / (float)TILES_PER_ROW + (1 / (float)REAL_TEX_SIZE)*0.001;

    *u1 = *u + 1 / (float)TILES_PER_ROW;
    *v1 = *v + 1 / (float)TILES_PER_ROW;

}


static void projection_orthographic(mod16Matrix_t * out,
    float left, float right,
    float top, float bottom,
    float zNear, float zFar
) {
    mod16_matrix_set_identity(out);
    float * projection = out->data+0;
    projection[0] = (2)  / (right - left);
    projection[5] = (2)  / (top - bottom);
    projection[10] = (-2) / (zFar - zNear);

    projection[3]  = -(right + left) / (right - left);
    projection[7]  = -(top + bottom) / (top - bottom);
    projection[11] = -(zFar + zNear) / (zFar - zNear);

}









static void mod16_sdl_gl_render_background_batch(mod16SDLGL_t * gl, MOD16_GLBackgroundBatch * batch) {

    // depth
    switch(batch->effect) {
      case RE_COLOR:
      case RE_MASK:
      case RE_BLEND:
      case RE_MASK_AND_COLOR:
        glDepthFunc(GL_ALWAYS);
        break;
        
      case RE_COLOR_ON_MASK:
      case RE_BLEND_ON_MASK:
        glDepthFunc(GL_LESS);
        break;
      case RE_COLOR_AROUND_MASK:
      case RE_BLEND_AROUND_MASK:
        glDepthFunc(GL_EQUAL);
        break;
    }
    
    
    // blending
    switch(batch->effect) {
      case RE_BLEND:
      case RE_BLEND_ON_MASK:
      case RE_BLEND_AROUND_MASK:
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE);
        break;
      default:
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    }
    

    
    
    glUniform1f(gl->spriteProgram.locationUniformEffect, (float)batch->effect);
    glUniform1f(gl->spriteProgram.locationUniformTexture, 1.f);
    glUniform1f(gl->spriteProgram.locationUniformUseStaticColors, 0.f);

    glActiveTexture(GL_TEXTURE0);



    
    glBindTexture(GL_TEXTURE_2D, batch->texture);


    glBindBuffer(GL_ARRAY_BUFFER, gl->spriteProgram.vbo);
    MOD16_VBOvertex base = {batch->x0, batch->y0, 0, 0, 0, batch->back.x, batch->back.y, batch->back.z,      batch->midBack.x, batch->midBack.y, batch->midBack.z,       batch->midFront.x, batch->midFront.y, batch->midFront.z,        batch->front.x, batch->front.y, batch->front.z,  batch->top.x, batch->top.y, batch->top.z, 1.f, 1.f, 1.f};

    MOD16_VBOvertex vboData[] = {
        base,
        base,
        base,
        base,
        base,
        base
    };
    
    vboData[1].x = batch->x1;
    vboData[1].y = batch->y1;
    vboData[1].u = 1;    
    vboData[1].v = 0;    

    vboData[2].x = batch->x2;
    vboData[2].y = batch->y2;
    vboData[2].u = 1;    
    vboData[2].v = 1;    

    vboData[3].x = batch->x2;
    vboData[3].y = batch->y2;
    vboData[3].u = 1;    
    vboData[3].v = 1;    

    vboData[4].x = batch->x3;
    vboData[4].y = batch->y3;
    vboData[4].u = 0;    
    vboData[4].v = 1;    


    

    glBufferData(GL_ARRAY_BUFFER, sizeof(MOD16_VBOvertex)*6, vboData, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(gl->spriteProgram.locationVBOposition, 3, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)0);
    glVertexAttribPointer(gl->spriteProgram.locationVBOuv,       2, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)(sizeof(float)*3));
    glVertexAttribPointer(gl->spriteProgram.locationVBOback,     3, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)(sizeof(float)*5));
    glVertexAttribPointer(gl->spriteProgram.locationVBOmidBack,  3, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)(sizeof(float)*8));
    glVertexAttribPointer(gl->spriteProgram.locationVBOmidFront, 3, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)(sizeof(float)*11));
    glVertexAttribPointer(gl->spriteProgram.locationVBOfront,    3, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)(sizeof(float)*14));
    glVertexAttribPointer(gl->spriteProgram.locationVBOtop,      3, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)(sizeof(float)*17));
    glVertexAttribPointer(gl->spriteProgram.locationVBObase,     3, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)(sizeof(float)*20));

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
}


static void mod16_sdl_gl_render_vertex_batch(mod16SDLGL_t * gl, MOD16_GLVertexBatch * batch) {

    // depth
    switch(batch->effect) {
      case RE_COLOR:
      case RE_MASK:
      case RE_BLEND:
      case RE_MASK_AND_COLOR:
        glDepthFunc(GL_ALWAYS);
        break;
        
      case RE_COLOR_ON_MASK:
      case RE_BLEND_ON_MASK:
        glDepthFunc(GL_LESS);
        break;
      case RE_COLOR_AROUND_MASK:
      case RE_BLEND_AROUND_MASK:
        glDepthFunc(GL_EQUAL);
        break;
    }
    
    
    // blending
    switch(batch->effect) {
      case RE_BLEND:
      case RE_BLEND_ON_MASK:
      case RE_BLEND_AROUND_MASK:
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE);
        break;
      default:
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    }
    

    
    
    glUniform1f(gl->spriteProgram.locationUniformEffect, (float)batch->effect);
    glActiveTexture(GL_TEXTURE0);

    mod16Matrix_t m = mod16_matrix_multiply(
        &gl->proj,
        &batch->transform
    );

    glUniformMatrix4fv(
        gl->spriteProgram.locationUniformProj,
        1, // count,
        1, // transpose (yes)
        (GLfloat*)&m
    );

    
    glUniform1f(gl->spriteProgram.locationUniformTexture, batch->texture > -1 ? 1.f : 0.f);
    glUniform1f(gl->spriteProgram.locationUniformUseStaticColors, 1.f);
    glBindTexture(GL_TEXTURE_2D, batch->texture);

    glUniform3f(gl->spriteProgram.locationUniformBackStatic, batch->back.x, batch->back.y, batch->back.z);
    glUniform3f(gl->spriteProgram.locationUniformMidBackStatic, batch->midBack.x, batch->midBack.y, batch->midBack.z);
    glUniform3f(gl->spriteProgram.locationUniformMidFrontStatic, batch->midFront.x, batch->midFront.y, batch->midFront.z);
    glUniform3f(gl->spriteProgram.locationUniformFrontStatic, batch->front.x, batch->front.y, batch->front.z);
    glUniform3f(gl->spriteProgram.locationUniformTopStatic, batch->top.x, batch->top.y, batch->top.z);




    glBindBuffer(GL_ARRAY_BUFFER, batch->vbo);
    glVertexAttribPointer(gl->spriteProgram.locationVBOposition, 3, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)0);
    glVertexAttribPointer(gl->spriteProgram.locationVBOuv,       2, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)(sizeof(float)*3));
    glVertexAttribPointer(gl->spriteProgram.locationVBOback,     3, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)(sizeof(float)*5));
    glVertexAttribPointer(gl->spriteProgram.locationVBOmidBack,  3, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)(sizeof(float)*8));
    glVertexAttribPointer(gl->spriteProgram.locationVBOmidFront, 3, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)(sizeof(float)*11));
    glVertexAttribPointer(gl->spriteProgram.locationVBOfront,    3, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)(sizeof(float)*14));
    glVertexAttribPointer(gl->spriteProgram.locationVBOtop,      3, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)(sizeof(float)*17));
    glVertexAttribPointer(gl->spriteProgram.locationVBObase,     3, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)(sizeof(float)*20));


    glDrawArrays(batch->shape == 0 ? GL_TRIANGLES : GL_LINES, 0, batch->count);
    glBindTexture(GL_TEXTURE_2D, 0);
}



static void mod16_sdl_gl_render_sprite_batch(mod16SDLGL_t * gl, MOD16_GLSpriteBatch * batch) {

    // depth
    switch(batch->effect) {
      case RE_COLOR:
      case RE_MASK:
      case RE_BLEND:
      case RE_MASK_AND_COLOR:
        glDepthFunc(GL_ALWAYS);
        break;
        
      case RE_COLOR_ON_MASK:
      case RE_BLEND_ON_MASK:
        glDepthFunc(GL_LESS);
        break;
      case RE_COLOR_AROUND_MASK:
      case RE_BLEND_AROUND_MASK:
        glDepthFunc(GL_EQUAL);
        break;
    }
    
    
    // blending
    switch(batch->effect) {
      case RE_BLEND:
      case RE_BLEND_ON_MASK:
      case RE_BLEND_AROUND_MASK:
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE);
        break;
      default:
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    }

    glUniform1f(gl->spriteProgram.locationUniformEffect, batch->effect);
    glUniform1f(gl->spriteProgram.locationUniformTexture, 1.f);
    glUniform1f(gl->spriteProgram.locationUniformUseStaticColors, 0.f);

    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, batch->texture);
    glBindBuffer(GL_ARRAY_BUFFER, gl->spriteProgram.vbo);

    
    glBufferData(
        GL_ARRAY_BUFFER, 
        sizeof(MOD16_VBOvertex)*matte_array_get_size(batch->vertices), 
        matte_array_get_data(batch->vertices), 
        GL_DYNAMIC_DRAW
    );

    glVertexAttribPointer(gl->spriteProgram.locationVBOposition, 3, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)0);
    glVertexAttribPointer(gl->spriteProgram.locationVBOuv,       2, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)(sizeof(float)*3));
    glVertexAttribPointer(gl->spriteProgram.locationVBOback,     3, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)(sizeof(float)*5));
    glVertexAttribPointer(gl->spriteProgram.locationVBOmidBack,  3, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)(sizeof(float)*8));
    glVertexAttribPointer(gl->spriteProgram.locationVBOmidFront, 3, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)(sizeof(float)*11));
    glVertexAttribPointer(gl->spriteProgram.locationVBOfront,    3, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)(sizeof(float)*14));
    glVertexAttribPointer(gl->spriteProgram.locationVBOtop,      3, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)(sizeof(float)*17));
    glVertexAttribPointer(gl->spriteProgram.locationVBObase,     3, GL_FLOAT, GL_FALSE, sizeof(MOD16_VBOvertex), (void*)(sizeof(float)*20));

    glDrawArrays(GL_TRIANGLES, 0, matte_array_get_size(batch->vertices));
    glBindTexture(GL_TEXTURE_2D, 0);

}



static int mod16_sdl_gl_get_error() {
    return glGetError();
}



static MOD16_GLFramebuffer create_framebuffer(int w, int h) {
    MOD16_GLFramebuffer fb = {};
    glGenFramebuffers(1, &fb.handle);
    glGenRenderbuffers(1, &fb.depthRB);
    glGenTextures(1, &fb.texture);
    
    glBindTexture(GL_TEXTURE_2D, fb.texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        w,
        h,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        NULL    
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_NEAREST
    );
    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_NEAREST
    );
    glBindTexture(GL_TEXTURE_2D, 0);
    
    
    glBindRenderbuffer(GL_RENDERBUFFER, fb.depthRB);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, w, h);    
    
    glBindFramebuffer(GL_FRAMEBUFFER, fb.handle);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb.texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_RENDERBUFFER, fb.depthRB);
    
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("Framebuffer operation could not be completed.\n");
        exit(1);
    }
    
    fb.width = w;
    fb.height = h;
    return fb;        
}






