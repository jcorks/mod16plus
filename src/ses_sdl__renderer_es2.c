#include "matte/src/matte_array.h"
#include "linear.h"
#include <GLES2/gl2.h>
#include <SDL2/SDL.h>


typedef struct {
    GLuint handle;
    GLuint depthRB;
    GLuint texture;
    int width;
    int height;
} SES_GLFramebuffer;


typedef struct {
    GLuint handle;
    GLuint vertexShader;
    GLuint fragmentShader;
    
    GLint locationVBOposition;
    GLint locationVBOuv;
    
    GLint locationUniformLocalMat;
    GLint locationUniformProj;
    GLint locationUniformEffect;
    GLint locationUniformPalette;
    
    GLint vbo;
} SES_GLProgram;

typedef struct {
    float x;
    float y;
    float u;
    float v;
} SES_VBOvertex;


typedef struct {
    uint8_t data[64];
} SES_GLTile;

typedef struct {
    GLint texture;
    SES_GLTile tiles[4096];
} SES_GLTileTexture;


typedef struct {

    
    // The raw framebuffer that holds the low-resolution
    // results. This will be of the user's requested size
    SES_GLFramebuffer resultFramebuffer;
    SES_GLProgram spriteProgram;
    SES_GLProgram screenProgram;
    matteArray_t * textures;
    
    int resolutionWidth;
    int resolutionHeight;

} SES_GLRenderer;


static SES_GLRenderer gl;



static SES_GLFramebuffer create_framebuffer(int w, int h) {
    SES_GLFramebuffer fb = {};
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
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);    
    
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


#include "shaders/ses_sdl__renderer_es2__sprite_frag_data"
#include "shaders/ses_sdl__renderer_es2__sprite_vtx_data"
#include "shaders/ses_sdl__renderer_es2__screen_frag_data"
#include "shaders/ses_sdl__renderer_es2__screen_vtx_data"

static SES_GLProgram create_program(const char * srcVertex, const char * srcFragment) {
    SES_GLProgram program = {};
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
        printf("ES2: Vertex shader failed to compile. Log:\n%s\n", log);
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
        printf("ES2: Fragment shader failed to compile. Log:\n%s\n", log);
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
        printf("ES2: Program failed to link. Log:\n%s\n", log);
        free(log);
        exit(112);

    }

    glDeleteShader(program.vertexShader);
    glDeleteShader(program.fragmentShader);
    
    
    program.locationVBOposition = glGetAttribLocation(program.handle, "position");
    program.locationVBOuv = glGetAttribLocation(program.handle, "uv");

    program.locationUniformEffect = glGetAttribLocation(program.handle, "effect");
    program.locationUniformLocalMat = glGetAttribLocation(program.handle, "localMat");
    program.locationUniformPalette = glGetAttribLocation(program.handle, "palette");
    program.locationUniformProj = glGetAttribLocation(program.handle, "proj");
    
    glGenBuffers(1, &program.vbo);
    
    return program;
    
}










////////// sprite handling 
/*
    Each sprite is a standalone 
    rendering.
    
    For now, we are doing one texture bind 
    + draw per sprite. Not fast, but simple 
    to implement. 
    
    Later, we can do texture atlasing + batching 

*/









////////// interface

void ses_sdl_gl_init(SDL_Window ** window, SDL_GLContext ** context) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);    
    
    *window  = SDL_CreateWindow("Sprite Entertainment System", 100, 100, 800, 600, SDL_WINDOW_OPENGL);
    *context = SDL_GL_CreateContext(*window);
    
    // default is gba resolution    
    gl.resultFramebuffer = create_framebuffer(240, 160);
    gl.resolutionWidth = 240;
    gl.resolutionHeight = 160;

    gl.spriteProgram = create_program(
        ses_sdl__renderer_es2__sprite_vtx_data,
        ses_sdl__renderer_es2__sprite_frag_data
    );
    
    gl.screenProgram = create_program(
        ses_sdl__renderer_es2__screen_vtx_data,
        ses_sdl__renderer_es2__screen_frag_data
    );
    
    
    
    gl.textures = matte_array_create(sizeof(SES_GLTileTexture*));
}


#define REAL_TEX_WIDTH  512
#define REAL_TEX_HEIGHT 512

static uint8_t NO_TILE_DATA[64] = {};

static SES_GLTileTexture * bound_tile_texture = NULL;
static uint32_t            bound_tile_id = 0;
static int                 bound_tile_dirty = 0;

void ses_sdl_gl_bind_tile(uint32_t id) {
    // 512 x 512 per GL texture, which means 
    // each texture holds 4096 tiles.
    int slot = id / 4096;
    int index = id % 4096;
    
    while(matte_array_get_size(gl.textures) <= slot) {
        SES_GLTileTexture * t = NULL;
        matte_array_push(gl.textures, t);
    }
    SES_GLTileTexture * t = matte_array_at(gl.textures, SES_GLTileTexture *, slot);
    if (t == NULL) {
        t = calloc(1, sizeof(SES_GLTileTexture));
        glGenTextures(1, &t->texture);
        glBindTexture(GL_TEXTURE_2D, t->texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_ALPHA,
            512,
            512,
            0,
            GL_ALPHA,
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
        matte_array_at(gl.textures, SES_GLTileTexture *, slot) = t;
    }
    
    bound_tile_id = id;
    bound_tile_dirty = 0;
    bound_tile_texture = t;     
}
void ses_sdl_gl_set_tile_pixel(uint8_t location, uint8_t value) {
    bound_tile_texture->tiles[bound_tile_id%4096].data[location] = value;
    bound_tile_dirty = 1;
};

uint8_t ses_sdl_gl_get_tile_pixel(uint8_t location) {
    return bound_tile_texture->tiles[bound_tile_id%4096].data[location];
}

void ses_sdl_gl_unbind_tile() {
    if (bound_tile_dirty) {
        int index = bound_tile_id % 4096;


        glBindTexture(GL_TEXTURE_2D, bound_tile_texture->texture);
        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            (index % 64)*8,
            (index / 64)*8,
            8,
            8,
            GL_ALPHA,
            GL_UNSIGNED_BYTE,
            bound_tile_texture->tiles[bound_tile_id%4096].data // always 64 bytes
        );
        glBindTexture(GL_TEXTURE_2D, 0);     
    }
    bound_tile_texture = NULL;
}




static GLint ses_sdl_gl_get_tile_attribs(uint32_t id, float * u, float * v, float * unit) {
    *u = 0;
    *v = 0;
    int slot = id / 4096;
    int index = id % 4096;
    if (slot >= matte_array_get_size(gl.textures))
        return 1;
        
    SES_GLTileTexture * t = matte_array_at(gl.textures, SES_GLTileTexture *, id);
    if (t == NULL)
        return 1;
        
    *u = (index % 64)   / 64.0;
    *v = (index / 64.0) / 64.0;
    *unit = 1 / 64.0;
    return t->texture;
}


static void projection_orthographic(sesMatrix_t * out,
    float left, float right,
    float top, float bottom,
    float zNear, float zFar
) {
    ses_matrix_set_identity(out);
    float * projection = out->data+0;
    projection[0] = (2)  / (right - left);
    projection[5] = (2)  / (top - bottom);
    projection[10] = (-2) / (zFar - zNear);

    projection[3]  = -(right + left) / (right - left);
    projection[7]  = -(top + bottom) / (top - bottom);
    projection[11] = -(zFar + zNear) / (zFar - zNear);

}


typedef enum {
    RE_COLOR,
    RE_MASK,
    RE_MASK_AND_COLOR,
    RE_COLOR_ON_MASK,
    RE_COLOR_AROUND_MASK,
    RE_BLEND,
    RE_BLEND_ON_MASK,
    RE_BLEND_AROUND_MASK,
} RenderEffect;


void ses_sdl_gl_render_begin() {
    glUseProgram(gl.spriteProgram.handle);
    glBindFramebuffer(GL_FRAMEBUFFER, gl.resultFramebuffer.handle);
    glClearColor(0.12, 0.051, 0.145, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    sesMatrix_t proj;
    projection_orthographic(
        &proj, 
        -gl.resolutionWidth/2, gl.resolutionWidth/2,
        gl.resolutionHeight/2, -gl.resolutionHeight/2,
        100, -100
    );

    glUniformMatrix4fv(
        gl.spriteProgram.locationUniformProj,
        1, // count,
        1, // transpose (yes)
        (GLfloat*)&proj
    );
    
    
    glEnableVertexAttribArray(gl.spriteProgram.locationVBOposition);
    glEnableVertexAttribArray(gl.spriteProgram.locationVBOuv);
    
    glVertexAttribPointer(gl.spriteProgram.locationVBOposition, 2, GL_FLOAT, GL_FALSE, sizeof(SES_VBOvertex), (void*)0);
    glVertexAttribPointer(gl.spriteProgram.locationVBOuv,       2, GL_FLOAT, GL_FALSE, sizeof(SES_VBOvertex), (void*)(sizeof(float)*2));
    
    
}


// cleans up + draws to main framebuffer
void ses_sdl_gl_render_end() {
    glDisableVertexAttribArray(gl.spriteProgram.locationVBOposition);
    glDisableVertexAttribArray(gl.spriteProgram.locationVBOuv);

    glUseProgram(gl.screenProgram.handle);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    glClearColor(1, 0, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    SES_VBOvertex vboData[] = {
        {0, 0, 0, 0},
        {1, 0, 1, 0},
        {1, 1, 1, 1},

        {1, 1, 1, 1},
        {0, 1, 0, 1},
        {0, 0, 0, 0}
    };    

    glEnableVertexAttribArray(gl.screenProgram.locationVBOposition);
    glEnableVertexAttribArray(gl.screenProgram.locationVBOuv);
    
    glVertexAttribPointer(gl.screenProgram.locationVBOposition, 2, GL_FLOAT, GL_FALSE, sizeof(SES_VBOvertex), (void*)0);
    glVertexAttribPointer(gl.screenProgram.locationVBOuv,       2, GL_FLOAT, GL_FALSE, sizeof(SES_VBOvertex), (void*)(sizeof(float)*2));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gl.resultFramebuffer.texture);
    glBindBuffer(GL_ARRAY_BUFFER, gl.screenProgram.vbo);
    

    glBufferData(GL_ARRAY_BUFFER, sizeof(SES_VBOvertex)*6, vboData, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);


    glDisableVertexAttribArray(gl.screenProgram.locationVBOposition);
    glDisableVertexAttribArray(gl.screenProgram.locationVBOuv);
}

void ses_sdl_gl_render_sprite(
    float x, float y,
    float scaleX, float scaleY,
    float centerX, float centerY,
    float rotation,
    int layer,
    int effect,

    sesVector_t back,
    sesVector_t midBack,
    sesVector_t midFront,
    sesVector_t front,
    
    uint32_t id
) {
    glEnable(GL_DEPTH_TEST);

    // depth
    switch(effect) {
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
    switch(effect) {
      case RE_BLEND:
      case RE_BLEND_ON_MASK:
      case RE_BLEND_AROUND_MASK:
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE);
        break;
      default:
        glDisable(GL_BLEND);
    }
    

    // order:
    /*
        - center translation
        - scale
        - rotation 
        - position
    */
    
    sesMatrix_t m;
    ses_matrix_set_identity(&m);
    ses_matrix_translate(&m, centerX, centerY, 0);
    ses_matrix_scale(&m, scaleX, scaleY, 0);
    ses_matrix_rotate_by_angles(&m, 0, 0, rotation);
    ses_matrix_translate(&m, x, y, 0);
    
    

    
    glUniformMatrix4fv(
        gl.spriteProgram.locationUniformLocalMat,
        1, // count,
        1, // transpose (yes)
        (GLfloat*)&m
    );
    
    glUniform1i(gl.spriteProgram.locationUniformEffect, effect);
    
    float paletteReal[] = {
        0, 0, 0, 0,
        back.x, back.y, back.z, 0,
        midBack.x, midBack.y, midBack.z, 0,
        midFront.x, midFront.y, midFront.z, 0,
        front.x, front.y, front.z, 0
    };
    glUniform4fv(gl.spriteProgram.locationUniformPalette, 5, paletteReal);
    
    
    glActiveTexture(GL_TEXTURE0);

    float u, v, unit;        
    GLint tex = ses_sdl_gl_get_tile_attribs(id, &u, &v, &unit);
    glBindTexture(GL_TEXTURE_2D, tex);


    glBindBuffer(GL_ARRAY_BUFFER, gl.spriteProgram.vbo);
    SES_VBOvertex vboData[] = {
        {0, 0, u,      v},
        {1, 0, u+unit, v},
        {1, 1, u+unit, v+unit},

        {1, 1, u+unit, v+unit},
        {0, 1, u,      v+unit},
        {0, 0, u,      v}
    };
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(SES_VBOvertex)*6, vboData, GL_DYNAMIC_DRAW);


    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);

}



