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
    if (argc != 2)
        exit(1);
    uint32_t len = 0;
    uint8_t * bytes = dump_bytes(argv[1], &len);
    
    
    FILE * out = fopen("api_rom", "wb");
    fprintf(out,"static uint8_t API_ROM_DATA[] = {");
    uint32_t i;
    
    for(i = 0; i < len; ++i) {
        fprintf(out, "%d,", bytes[i]);
        if (i%32==0)
            fprintf(out,"\n");
    }
    
    fclose(out);
    return 0;
}
