#include "rom.h"
#include <string.h>
#include <stdlib.h>



typedef struct {
    uint32_t length;
    uint8_t * data;
} SESWaveform;

typedef struct {
    uint32_t id;
    uint8_t data[64];
} SESTile;

typedef struct {
    uint32_t id;
    float data[12];
} SESPalette;


typedef struct {
    matteString_t * name;
    uint32_t length;
    uint8_t * data;
} SESBytecodeSegment;

static matteArray_t * rom_waveforms = NULL;
static matteArray_t * rom_tiles = NULL;
static matteArray_t * rom_palettes = NULL;
static matteArray_t * rom_bytecodeSegments = NULL;




#define SES_CHOMP(__T__) *((__T__*)(romBytes+iter)); iter+=sizeof(__T__); if(iter > romLength) goto L_BAD_SIZE;

#define SES_CHOMP_LARGE(__N__, __DEST__) {\
  if (iter + (__N__) > romLength) goto L_BAD_SIZE;\
  memcpy(__DEST__, romBytes+iter, __N__);\
  iter+=__N__;\
}
  
  
static void clear_rom() {
    if (!rom_waveforms) {
        rom_waveforms = matte_array_create(sizeof(SESWaveform));
        rom_tiles = matte_array_create(sizeof(SESTile));
        rom_palettes = matte_array_create(sizeof(SESPalette));
        rom_bytecodeSegments = matte_array_create(sizeof(SESBytecodeSegment));
    }

    
    // clear existing ROM
    uint32_t i;
    uint32_t len;
    len = matte_array_get_size(rom_waveforms);
    for(i = 0; i < len; ++i) {
        free(matte_array_at(rom_waveforms, SESWaveform, i).data);
    }
    matte_array_set_size(rom_waveforms, 0);
    matte_array_set_size(rom_tiles, 0);
    matte_array_set_size(rom_palettes, 0);
    len = matte_array_get_size(rom_bytecodeSegments);
    for(i = 0; i < len; ++i) {
        matte_string_destroy(matte_array_at(rom_bytecodeSegments, SESBytecodeSegment, i).name);
        free(matte_array_at(rom_bytecodeSegments, SESBytecodeSegment, i).data);
    }
    matte_array_set_size(rom_bytecodeSegments, 0);
    

}

SESUnpackError_t ses_unpack_rom(const uint8_t * romBytes, uint32_t romLength) {
    clear_rom();

    uint32_t i, len;


    // before starting, check the size.
    if (romLength < 12 + 4*sizeof(uint32_t)) 
        return SES_UNPACK_ERROR__TOO_SMALL;
    
    
    // first, check header 
    uint32_t iter = 0;
    if (!(
        romBytes[0] == 'S' &&  romBytes[1] == ' ' &&
        romBytes[2] == 'E' &&  romBytes[3] == ' ' &&
        romBytes[4] == 'S' &&  romBytes[5] == ' ' &&
        romBytes[6] == 'J' &&  romBytes[7] == ' ' &&
        romBytes[8] == 'L' &&  romBytes[9] == ' ' &&
        romBytes[10] == 'C' && romBytes[11] == ' '
    )) {
        return SES_UNPACK_ERROR__BAD_HEADER;
    }
    
    if (romBytes[12] != 1) 
        return SES_UNPACK_ERROR__UNSUPPORTED_VERSION;
    iter += 13;
        
        
    uint32_t waveformCount        = SES_CHOMP(uint32_t);
    uint32_t tileCount            = SES_CHOMP(uint32_t);
    uint32_t paletteCount         = SES_CHOMP(uint32_t);
    uint32_t bytecodeSegmentCount = SES_CHOMP(uint32_t);
    
    
    
    // waveforms
    for(i = 0; i < waveformCount; ++i) {
        SESWaveform wave;
        wave.length = SES_CHOMP(uint32_t);
        wave.data = malloc(wave.length);
        
        SES_CHOMP_LARGE(wave.length, wave.data);
        matte_array_push(rom_waveforms, wave);
    }
    
    // tiles 
    matte_array_set_size(rom_tiles, tileCount);
    SES_CHOMP_LARGE(
        tileCount * sizeof(SESTile),
        matte_array_get_data(rom_tiles)
    );
    

    // palettes
    matte_array_set_size(rom_tiles, paletteCount);
    SES_CHOMP_LARGE(
        paletteCount * sizeof(SESPalette),
        matte_array_get_data(rom_palettes)
    );



    // bytecodeSegments
    for(i = 0; i < bytecodeSegmentCount; ++i) {
        SESBytecodeSegment seg;
        seg.length = SES_CHOMP(uint32_t);        
        uint32_t nameLenBytes = SES_CHOMP(uint32_t);
        char * name = malloc(nameLenBytes+1);
        SES_CHOMP_LARGE(nameLenBytes, name);
        name[nameLenBytes] = 0;
        seg.name = matte_string_create_from_c_str("%s", name);
        free(name);
        seg.data = malloc(seg.length);
        SES_CHOMP_LARGE(seg.length, seg.data);
        matte_array_push(rom_bytecodeSegments, seg);
    }
    
    
    return 0;         
    

    // chomp can send
  L_BAD_SIZE:
    return SES_UNPACK_ERROR__SIZE_MISMATCH;
  
    
}



#define PUSHBYTE(__V__) {uint8_t g = __V__; matte_array_push(bytes, g);}
#define PUSHCOPY(__T__, __V__) {__T__ g = __V__; uint32_t n; matte_array_push_n(bytes, (uint8_t*)&g, sizeof(__T__));}
#define PUSHN(__N__, __VP__) {uint32_t n; matte_array_push_n(bytes, __VP__, __N__);}
matteArray_t * ses_pack_rom(
    matteArray_t * waveformSizes, // uint32_t
    matteArray_t * waveforms, // uint8_t *     
    
    matteArray_t * tileIDs,
    matteArray_t * tiles, // uint8_t, see ses_rom_get_tile

    matteArray_t * paletteIDs,
    matteArray_t * palettes, // float, see ses_rom_get_palette
    
    
    matteArray_t * bytecodeSegmentNames, // matteString_t *
    matteArray_t * bytecodeSegmentSizes, // uint32_t
    matteArray_t * bytecodeSegments // uint8_t *
) {
    clear_rom();

    matteArray_t * bytes = matte_array_create(sizeof(uint8_t));
    PUSHBYTE('S');
    PUSHBYTE(' ');
    PUSHBYTE('E');
    PUSHBYTE(' ');
    PUSHBYTE('S');
    PUSHBYTE(' ');
    PUSHBYTE('J');
    PUSHBYTE(' ');
    PUSHBYTE('L');
    PUSHBYTE(' ');
    PUSHBYTE('C');
    PUSHBYTE(' ');
    PUSHBYTE(1);


    PUSHCOPY(uint32_t, matte_array_get_size(waveforms));
    PUSHCOPY(uint32_t, matte_array_get_size(tiles));
    PUSHCOPY(uint32_t, matte_array_get_size(palettes));
    PUSHCOPY(uint32_t, matte_array_get_size(bytecodeSegments));


    uint32_t i, len;
    
    // waveforms
    len = matte_array_get_size(waveforms);
    for(i = 0; i < len; ++i) {
        uint32_t length = matte_array_at(waveformSizes, uint32_t, i);
        uint8_t * data = matte_array_at(waveforms, uint8_t *, i);

        PUSHCOPY(uint32_t, length);
        PUSHN(length, data);
    }
    
    // tiles 
    len = matte_array_get_size(tiles);
    for(i = 0; i < len; ++i) {
        SESTile tile;
        tile.id = matte_array_at(tileIDs, uint32_t, i);
        memcpy(tile.data, &matte_array_at(tiles, uint8_t, i*64), 64);
        PUSHN(
            sizeof(SESTile),
            &tile
        );
    };
    

    // palettes
    len = matte_array_get_size(palettes);
    for(i = 0; i < len; ++i) {
        SESPalette palette;
        palette.id = matte_array_at(paletteIDs, uint32_t, i);
        memcpy(palette.data, &matte_array_at(palettes, float, i*12), 12);
        PUSHN(
            sizeof(SESPalette),
            &palette
        );
    };


    // bytecodeSegments
    len = matte_array_get_size(bytecodeSegments);
    for(i = 0; i < len; ++i) {
        uint32_t length = matte_array_at(bytecodeSegmentSizes, uint32_t, i);
        PUSHCOPY(uint32_t, length);

        matteString_t * name = matte_array_at(bytecodeSegmentNames, matteString_t *, i);
        uint32_t nameLen = matte_string_get_byte_length(name);
        PUSHCOPY(uint32_t, nameLen);
        
        PUSHN(
            nameLen,
            matte_string_get_byte_data(name)
        );


        PUSHN(
            length,
            matte_array_at(bytecodeSegments, uint8_t *, i)
        );
    }
    
    return bytes;
    

}



uint32_t ses_rom_get_waveform_count() {
    if (!rom_waveforms) return 0;
    return matte_array_get_size(rom_waveforms);
}

const uint8_t * ses_rom_get_waveform(uint32_t id, uint32_t * lengthBytes) {
    *lengthBytes = 0;
    if (!rom_waveforms) return NULL;
    if (id >= ses_rom_get_waveform_count()) return NULL;

    SESWaveform wave = matte_array_at(rom_waveforms, SESWaveform, id);
    *lengthBytes = wave.length;
    return wave.data;
}






uint32_t ses_rom_get_tile_count() {
    if (!rom_tiles) return 0;
    return matte_array_get_size(rom_tiles);
}

const uint8_t * ses_rom_get_tile(uint32_t index, uint32_t * id) {
    if (!rom_tiles) return NULL;
    if (index >= ses_rom_get_tile_count()) return NULL;
    
    SESTile * tile = &matte_array_at(rom_tiles, SESTile, index);
    *id = tile->id;
    return tile->data;
}



uint32_t ses_rom_get_palette_count() {
    if (!rom_palettes) return 0;
    return matte_array_get_size(rom_palettes);
}

const float * ses_rom_get_palette(uint32_t index, uint32_t * id) {
    if (!rom_palettes) return NULL;
    if (index >= ses_rom_get_palette_count()) return NULL;
    

    SESPalette * palette = &matte_array_at(rom_tiles, SESPalette, index);
    *id = palette->id;
    return palette->data;
}






uint32_t ses_rom_get_bytecode_segment_count() {
    if (!rom_bytecodeSegments) return 0;
    return matte_array_get_size(rom_bytecodeSegments);
}

const uint8_t * ses_rom_get_bytecode_segment(uint32_t id, uint32_t * length, matteString_t * name) {
    *length = 0;
    if (!rom_bytecodeSegments) return NULL;
    if (id >= ses_rom_get_bytecode_segment_count()) return NULL;

    SESBytecodeSegment seg = matte_array_at(rom_bytecodeSegments, SESBytecodeSegment, id);
    *length = seg.length;
    matte_string_set(name, seg.name);
    return seg.data;
}







