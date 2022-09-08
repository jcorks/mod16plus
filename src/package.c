#include "package.h"
#include "dump.h"
#include "rom.h"
#include "matte/src/matte_string.h"
#include "matte/src/matte_array.h"
#include "matte/src/matte_compiler.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// given bytes to a text file, returns an array of matteString_t * of 
// each line.
static matteArray_t * package_split(const char * dir, const char * sub) {
    uint8_t * bytes;
    uint32_t len;

    matteString_t * sourceStr = matte_string_create_from_c_str("%s/%s", dir, sub);    
    bytes = dump_bytes(matte_string_get_c_str(sourceStr), &len);
    matte_string_destroy(sourceStr);

    char * str = malloc(len+1);
    memcpy(str, bytes, len);
    str[len] = 0;
    
    matteString_t * text = matte_string_create_from_c_str("%s", str);
    matteString_t * line = matte_string_create();
    matteArray_t * arr = matte_array_create(sizeof(matteString_t *));
    free(str);
    
    uint32_t i;
    len = matte_string_get_length(text);
    for(i = 0; i < len; ++i) {
        uint32_t ch = matte_string_get_char(text, i);
        if (ch == '\n') {
            matte_array_push(arr, line);
            line = matte_string_create();        
        }
        matte_string_append_char(line, ch);
    }
    matte_array_push(arr, line);
    free(bytes);    
    return arr;    
}

static matteString_t * currentCompiled = NULL;
static void ses_package__compile_error(
    const matteString_t * str,
    uint32_t line, 
    uint32_t ch,
    void * userdata
) {
    printf(":::!!:::\nError in %s, line %d:%d\n\n%s\n:::!!:::\n", currentCompiled == NULL ? "???" : matte_string_get_c_str(currentCompiled), (int)line, (int)ch, matte_string_get_c_str(str));
    exit(1);
}


static void * dump_bytes_relative(const char * dir, const char * name, uint32_t * len) {
    matteString_t * path = matte_string_create_from_c_str("%s/%s", dir, name);
    void * out = dump_bytes(matte_string_get_c_str(path), len);
    matte_string_destroy(path);
    return out;
}

static int is_string_empty(const matteString_t * line) {
    uint32_t len = matte_string_get_length(line);
    if (len == 0) return 1;
    uint32_t i;
    
    for(i = 0; i < len; ++i) {
        if (matte_string_get_char(line, i) != ' ' &&
            matte_string_get_char(line, i) != '\n' &&
            matte_string_get_char(line, i) != '\t')
            return 0;
    }
    return 1;
}


int ses_package(const char * dir) {
    
    // for each source, dump
    uint32_t len;
    uint8_t * bytes;
    
    matteArray_t * waveformSizes = matte_array_create(sizeof(uint32_t));
    matteArray_t * waveforms = matte_array_create(sizeof(uint8_t*));     
    
    matteArray_t * tiles = matte_array_create(sizeof(uint8_t)*64); // uint8_t *, see ses_rom_get_tile
    matteArray_t * palettes = matte_array_create(sizeof(uint8_t)*12); // uint8_t *, see ses_rom_get_palette
    matteArray_t * backgrounds = matte_array_create(sizeof(uint32_t)*64); // uint32_t *, see ses_rom_get_background
    
    
    matteArray_t * bytecodeSegmentNames = matte_array_create(sizeof(matteString_t*));
    matteArray_t * bytecodeSegmentSizes = matte_array_create(sizeof(uint32_t));
    matteArray_t * bytecodeSegments = matte_array_create(sizeof(uint8_t*));
    
    // waveforms 
    {
        matteArray_t * lines = package_split(dir, "WAVEFORMS");
        uint32_t i;
        len = matte_array_get_size(lines);
        for(i = 0; i < len; ++i) {
            matteString_t * line = matte_array_at(lines, matteString_t *, i);
            if (is_string_empty(line)) {
                matte_string_destroy(line);
                continue;
            }
            uint32_t byteLen;
            bytes = dump_bytes_relative(
                dir,
                matte_string_get_c_str(line), 
                &byteLen
            );
            
            // raw waveforms (for now)
            // todo: possibly ogg
            matte_array_push(waveforms, bytes);
            matte_array_push(waveformSizes, byteLen);
        }        
    }
    

    // tiles 
    {
        matteArray_t * lines = package_split(dir, "TILES");
        uint32_t i;
        len = matte_array_get_size(lines);
        for(i = 0; i < len; ++i) {
            matteString_t * line = matte_array_at(lines, matteString_t *, i);
            if (is_string_empty(line)) {
                matte_string_destroy(line);
                continue;
            }
            uint32_t byteLen;
            bytes = dump_bytes_relative(
                dir,
                matte_string_get_c_str(line), 
                &byteLen
            );
            if (byteLen % 64 != 0) {
                printf("Tile sheet %s is misaligned and does not contain a multiple of 64 bytes.\n", matte_string_get_c_str(line));
                exit(1);
            }
            // raw waveforms (for now)
            // todo: possibly ogg
            matte_array_push_n(tiles, bytes, byteLen/64);
            matte_string_destroy(line);
            
        }        
    }

    // tiles 
    {
        matteArray_t * lines = package_split(dir, "PALETTES");
        uint32_t i;
        len = matte_array_get_size(lines);
        for(i = 0; i < len; ++i) {
            matteString_t * line = matte_array_at(lines, matteString_t *, i);
            if (is_string_empty(line)) {
                matte_string_destroy(line);
                continue;
            }
            uint32_t byteLen;
            bytes = dump_bytes_relative(
                dir,
                matte_string_get_c_str(line), 
                &byteLen
            );
            if (byteLen % 12 != 0) {
                printf("Palette sheet %s is misaligned and does not contain a multiple of 12 bytes.\n", matte_string_get_c_str(line));
                exit(1);
            }
            // raw waveforms (for now)
            // todo: possibly ogg
            matte_array_push_n(palettes, bytes, byteLen/12);
            matte_string_destroy(line);

        }        
    }    


    // tiles 
    {
        matteArray_t * lines = package_split(dir, "PALETTES");
        uint32_t i;
        len = matte_array_get_size(lines);
        for(i = 0; i < len; ++i) {
            matteString_t * line = matte_array_at(lines, matteString_t *, i);
            if (is_string_empty(line)) {
                matte_string_destroy(line);
                continue;
            }
            uint32_t byteLen;
            bytes = dump_bytes_relative(
                dir,
                matte_string_get_c_str(line), 
                &byteLen
            );
            if (byteLen % (64*sizeof(uint32_t)) != 0) {
                printf("background sheet %s is misaligned and does not contain a multiple of %d bytes.\n", matte_string_get_c_str(line), (int)(64*sizeof(uint32_t)));
                exit(1);
            }
            // raw waveforms (for now)
            // todo: possibly ogg
            matte_array_push_n(backgrounds, bytes, byteLen/(64*sizeof(uint32_t)));
            matte_string_destroy(line);

        }        
    }    
    
    // bytecode segments
    {
        matteArray_t * lines = package_split(dir, "SOURCES");
        uint32_t i;
        len = matte_array_get_size(lines);
        for(i = 0; i < len; ++i) {
            matteString_t * line = matte_array_at(lines, matteString_t *, i);
            if (is_string_empty(line)) {
                matte_string_destroy(line);
                continue;
            }
            uint32_t byteLen;
            bytes = dump_bytes_relative(
                dir,
                matte_string_get_c_str(line), 
                &byteLen
            );

            // now that we have a source, compile it
            uint32_t segmentLength;
            currentCompiled = line;
            bytes = matte_compiler_run(
                bytes,
                byteLen,
                &segmentLength,
                ses_package__compile_error, 
                NULL
            );
            currentCompiled = NULL;
            
            
            matte_array_push(bytecodeSegments, bytes);
            matte_array_push(bytecodeSegmentSizes, segmentLength);
            matte_array_push(bytecodeSegmentNames, line);
            
        }
    }
    
    matteArray_t * romBytes = ses_pack_rom(
        waveformSizes, // uint32_t
        waveforms, // uint8_t *     
        
        tiles, // uint8_t, see ses_rom_get_tile
        palettes, // uint8_t, see ses_rom_get_palette
        backgrounds, // uint32_t, see ses_rom_get_background
        
        
        bytecodeSegmentNames, // matteString_t *
        bytecodeSegmentSizes, // uint32_t
        bytecodeSegments // uint8_t *    
    );
    
    matteString_t * outname = matte_string_create_from_c_str("%s/%s", dir, "rom.ses");    

    return dump_file(matte_string_get_c_str(outname), matte_array_get_data(romBytes), matte_array_get_size(romBytes));
}

