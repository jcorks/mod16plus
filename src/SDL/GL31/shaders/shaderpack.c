#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
void * dump_bytes(const char * filename, uint32_t * len) {
    FILE * f = fopen(filename, "rb");
    if (!f) {
        printf("Could not open input file %s\n", filename);
        exit(1);    
    }
    char chunk[2048];
    int chunkSize;
    *len = 0;    
    while(chunkSize = (fread(chunk, 1, 2048, f))) *len += chunkSize;
    fseek(f, 0, SEEK_SET);


    void * out = malloc(*len);
    uint32_t iter = 0;
    while(chunkSize = (fread(chunk, 1, 2048, f))) {
        memcpy(out+iter, chunk, chunkSize);
        iter += chunkSize;
    }
    fclose(f);
    return out;
}

int main(int argc, char ** argv) {
    uint32_t len = 0;
    
    
    uint32_t NAME_COUNT = 4;
    char * NAMES_IN[] = {
        "mod16_sdl__renderer_gl31__sprite_vtx.glsl",    
        "mod16_sdl__renderer_gl31__sprite_frag.glsl",    
        "mod16_sdl__renderer_gl31__screen_vtx.glsl",    
        "mod16_sdl__renderer_gl31__screen_frag.glsl",    
    
    };
    
    char * NAMES_OUT[] = {
        "mod16_sdl__renderer_gl31__sprite_vtx_data",    
        "mod16_sdl__renderer_gl31__sprite_frag_data",    
        "mod16_sdl__renderer_gl31__screen_vtx_data",    
        "mod16_sdl__renderer_gl31__screen_frag_data",    
    
    };
    uint32_t i;
    for(i = 0; i < NAME_COUNT; ++i) {
        
        uint8_t * bytes = dump_bytes(NAMES_IN[i], &len);
        
        
        FILE * out = fopen(NAMES_OUT[i], "wb");
        fprintf(out, "static uint32_t %s_SIZE = %d;\n", NAMES_OUT[i], len);
        fprintf(out,"static uint8_t %s[] = {", NAMES_OUT[i]);
        uint32_t i;
        
        for(i = 0; i < len; ++i) {
            fprintf(out, "%d,", bytes[i]);
            if (i%32==0)
                fprintf(out,"\n");
        }
        fprintf(out, "0};\n"); // null terminated
        
        fclose(out);
    }
    return 0;
}
