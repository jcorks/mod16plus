#include "dump.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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


int dump_file(const char * filename, void * bytes, uint32_t len) {
    FILE * f = fopen(filename, "wb");
    if (f == NULL) return 0;
    
    if (fwrite(bytes, 1, len, f) != len) {
        fclose(f);
        return 0;
    }
    
    fclose(f);
    return 1;
}
