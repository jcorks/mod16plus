#include "rompack.h"
#include <string.h>
#include <stdlib.h>



typedef struct {
    uint32_t length;
    uint8_t * data;
} SESWaveform;

typedef struct {
    uint8_t data[64];
} SESTile;

typedef struct {
    uint8_t data[12];
} SESPalette;

typedef struct {
    uint32_t data[64];
} SESBackground;

typedef struct {
    matteString_t * name;
    uint32_t length;
    uint8_t * data;
} SESBytecodeSegment;

static matteArray_t * rom_waveforms = NULL;
static matteArray_t * rom_tiles = NULL;
static matteArray_t * rom_palettes = NULL;
static matteArray_t * rom_backgrounds = NULL;
static matteArray_t * rom_bytecodeSegments = NULL;




#define SES_CHOMP(__T__) *((__T__*)(romBytes+iter)); iter+=sizeof(__T__); if(iter >= romLength) goto L_BAD_SIZE;

#define SES_CHOMP_LARGE(__N__, __DEST__) {\
  if (iter + (__N__) >= romLength) goto L_BAD_SIZE;\
  memcpy(__DEST__, romBytes+iter, __N__);\
  iter+=__N__;\
}
  

SESUnpackError_t ses_unpack_rom(const uint8_t * romBytes, uint32_t romLength) {
    if (!rom_waveforms) {
        rom_waveforms = matte_array_create(sizeof(SESWaveform));
        rom_tiles = matte_array_create(sizeof(SESTile));
        rom_palettes = matte_array_create(sizeof(SESPalette));
        rom_backgrounds = matte_array_create(sizeof(SESBackground));
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
    matte_array_set_size(rom_backgrounds, 0);
    len = matte_array_get_size(rom_bytecodeSegments);
    for(i = 0; i < len; ++i) {
        matte_string_destroy(matte_array_at(rom_bytecodeSegments, SESBytecodeSegment, i).name);
        free(matte_array_at(rom_bytecodeSegments, SESBytecodeSegment, i).data);
    }
    matte_array_set_size(rom_bytecodeSegments, 0);
    


    // before starting, check the size.
    if (romLength < 12 + 5*sizeof(uint32_t)) 
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
    
    
    if (romBytes[13] != 1) 
        return SES_UNPACK_ERROR__UNSUPPORTED_VERSION;
    iter += 13;
        
        
    uint32_t waveformCount        = SES_CHOMP(uint32_t);
    uint32_t tileCount            = SES_CHOMP(uint32_t);
    uint32_t paletteCount         = SES_CHOMP(uint32_t);
    uint32_t backgroundCount      = SES_CHOMP(uint32_t);
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

    // backgrounds
    matte_array_set_size(rom_backgrounds, backgroundCount);
    SES_CHOMP_LARGE(
        backgroundCount * sizeof(SESBackground),
        matte_array_get_data(rom_backgrounds)
    );

    // bytecodeSegments
    for(i = 0; i < bytecodeSegmentCount; ++i) {
        SESBytecodeSegment seg;
        seg.length = SES_CHOMP(uint32_t);        
        uint32_t nameLenBytes = SES_CHOMP(uint32_t);
        char * name = malloc(seg.length+1);
        SES_CHOMP_LARGE(nameLenBytes, name);
        name[seg.length] = 0;
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

const uint8_t * ses_rom_get_tile(uint32_t id) {
    if (!rom_tiles) return NULL;
    if (id >= ses_rom_get_tile_count()) return NULL;
    
    return matte_array_at(rom_tiles, SESTile, id).data;
}



uint32_t ses_rom_get_palette_count() {
    if (!rom_palettes) return 0;
    return matte_array_get_size(rom_palettes);
}

const uint8_t * ses_rom_get_palette(uint32_t id) {
    if (!rom_palettes) return NULL;
    if (id >= ses_rom_get_palette_count()) return NULL;
    
    return matte_array_at(rom_palettes, SESPalette, id).data;
}



uint32_t ses_rom_get_background_count() {
    if (!rom_backgrounds) return 0;
    return matte_array_get_size(rom_backgrounds);
}
const uint32_t * ses_rom_get_background(uint32_t id) {
    if (!rom_backgrounds) return NULL;
    if (id >= ses_rom_get_background_count()) return NULL;
    
    return matte_array_at(rom_backgrounds, SESBackground, id).data;
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







