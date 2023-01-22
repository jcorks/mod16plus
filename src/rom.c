#include "rom.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


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

typedef struct {
    matteString_t * name;
    uint32_t length;
    uint8_t * data;
} SESSubCartridge;




struct sesROM_t {
    matteArray_t * waveforms;
    matteArray_t * tiles;
    matteArray_t * palettes;
    matteArray_t * bytecodeSegments;
    matteArray_t * subcartridges;
};



#define SES_CHOMP(__T__) *((__T__*)(romBytes+iter)); iter+=sizeof(__T__); if(iter > romLength) goto L_BAD_SIZE;

#define SES_CHOMP_LARGE(__N__, __DEST__) {\
  if (iter + (__N__) > romLength) goto L_BAD_SIZE;\
  memcpy(__DEST__, romBytes+iter, __N__);\
  iter+=__N__;\
}
  
  


sesROM_t * ses_rom_unpack(const uint8_t * romBytes, uint32_t romLength, sesROM_UnpackError_t * error) {
    sesROM_t * rom = calloc(1, sizeof(sesROM_t));
    *error = SES_ROM_UNPACK_NO_ERROR;

    rom->waveforms = matte_array_create(sizeof(SESWaveform));
    rom->tiles = matte_array_create(sizeof(SESTile));
    rom->palettes = matte_array_create(sizeof(SESPalette));
    rom->bytecodeSegments = matte_array_create(sizeof(SESBytecodeSegment));
    rom->subcartridges = matte_array_create(sizeof(SESSubCartridge));


    uint32_t i, len;


    // before starting, check the size.
    if (romLength < 12 + 4*sizeof(uint32_t)) {
        *error = SES_ROM_UNPACK_ERROR__TOO_SMALL;
        ses_rom_destroy(rom);
        return NULL;
    }    
    
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
        *error = SES_ROM_UNPACK_ERROR__BAD_HEADER;
        ses_rom_destroy(rom);
        return NULL;
    }
    
    if (romBytes[12] != 1) {
        *error = SES_ROM_UNPACK_ERROR__UNSUPPORTED_VERSION;
        ses_rom_destroy(rom);
        return NULL;
    };
    iter += 13;
        
        
    uint32_t waveformCount        = SES_CHOMP(uint32_t);
    uint32_t tileCount            = SES_CHOMP(uint32_t);
    uint32_t paletteCount         = SES_CHOMP(uint32_t);
    uint32_t bytecodeSegmentCount = SES_CHOMP(uint32_t);
    uint32_t subcartridgeCount    = SES_CHOMP(uint32_t);
    
    
    
    // waveforms
    for(i = 0; i < waveformCount; ++i) {
        SESWaveform wave;
        wave.length = SES_CHOMP(uint32_t);
        wave.data = malloc(wave.length);
        
        SES_CHOMP_LARGE(wave.length, wave.data);
        matte_array_push(rom->waveforms, wave);
    }
    
    // tiles 
    matte_array_set_size(rom->tiles, tileCount);
    SES_CHOMP_LARGE(
        tileCount * sizeof(SESTile),
        matte_array_get_data(rom->tiles)
    );
    

    // palettes
    matte_array_set_size(rom->tiles, paletteCount);
    SES_CHOMP_LARGE(
        paletteCount * sizeof(SESPalette),
        matte_array_get_data(rom->palettes)
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
        matte_array_push(rom->bytecodeSegments, seg);
    }

    // subcartridges
    for(i = 0; i < subcartridgeCount; ++i) {
        SESSubCartridge sub;
        sub.length = SES_CHOMP(uint32_t);        
        uint32_t nameLenBytes = SES_CHOMP(uint32_t);
        char * name = malloc(nameLenBytes+1);
        SES_CHOMP_LARGE(nameLenBytes, name);
        name[nameLenBytes] = 0;
        sub.name = matte_string_create_from_c_str("%s", name);
        free(name);
        sub.data = malloc(sub.length);
        SES_CHOMP_LARGE(sub.length, sub.data);
        matte_array_push(rom->subcartridges, sub);
    }

    
    
    return rom;         
    

    // chomp can send
  L_BAD_SIZE:
    ses_rom_destroy(rom);
    *error = SES_ROM_UNPACK_ERROR__SIZE_MISMATCH;
    return NULL;
  
    
}

sesROM_t * ses_rom_create(

    // Waveforms represent 16-bit signed integer, 2-channel PCM 
    // audio to be used for optional playback.
    // Waveforms cannot be made during runtime, so 
    // all the possible waveforms are represented here
    // in the ROM of the cartridge
    matteArray_t * waveformSizes, // uint32_t
    matteArray_t * waveforms, // uint8_t *     
    
    // Pre-computed tiles and their associated IDs. Tiles 
    // can be made during runtime, so this is just the initial 
    // set of tiles. IDs must be specified for each tile
    // Tiles are 64 bytes long.
    matteArray_t * tileIDs, // uint32_t
    matteArray_t * tiles, // uint8_t *, see ses_rom_get_tile


    // Pre-computed palettes and their associated IDs. 
    // Like tiles, palettes can be made during runtime, so 
    // these are just the initial set.
    // Palettes are 12 floating point values.
    matteArray_t * paletteIDs, // uint32_t
    matteArray_t * palettes, // uint8_t *, see ses_rom_get_palette
    
    // Pre-compiled sources and their associated names.    
    // Each bytecode segment is expected to be compiled Matte 
    // bytecode.
    matteArray_t * bytecodeSegmentNames, // matteString_t *
    matteArray_t * bytecodeSegmentSizes, // uint32_t
    matteArray_t * bytecodeSegments, // uint8_t *
    
    // Pre-compiled SES cartridge ROMs to be accessible as 
    // sub-cartridges. Each subcartridge is named, and each 
    // rom is assumed to have been made with ses_rom_pack() 
    matteArray_t * subcartridgeNames, // matteString_t *
    matteArray_t * subcartridgeROMSizes, // uint32_t 
    matteArray_t * subcartridgeROMs // uint8_t *
) {
    sesROM_t * rom = calloc(1, sizeof(sesROM_t));

    rom->waveforms = matte_array_create(sizeof(SESWaveform));
    rom->tiles = matte_array_create(sizeof(SESTile));
    rom->palettes = matte_array_create(sizeof(SESPalette));
    rom->bytecodeSegments = matte_array_create(sizeof(SESBytecodeSegment));
    rom->subcartridges = matte_array_create(sizeof(SESSubCartridge));


    uint32_t i, len;
    
    // waveforms
    len = matte_array_get_size(waveforms);
    for(i = 0; i < len; ++i) {
        uint32_t length = matte_array_at(waveformSizes, uint32_t, i);
        uint8_t * data = matte_array_at(waveforms, uint8_t *, i);

        SESWaveform wav = {};
        wav.length = length;
        wav.data = malloc(length);
        memcpy(wav.data, data, wav.length);
        
        matte_array_push(rom->subcartridges, wav);
    }
    
    // tiles 
    len = matte_array_get_size(tiles);
    for(i = 0; i < len; ++i) {
        SESTile tile;
        tile.id = matte_array_at(tileIDs, uint32_t, i);
        memcpy(tile.data, &matte_array_at(tiles, uint8_t, i*64), 64);

        matte_array_push(rom->tiles, tile);
    };
    

    // palettes
    len = matte_array_get_size(palettes);
    for(i = 0; i < len; ++i) {
        SESPalette palette;
        palette.id = matte_array_at(paletteIDs, uint32_t, i);
        memcpy(palette.data, &matte_array_at(palettes, float, i*12), 12);

        matte_array_push(rom->palettes, palette);
    };


    // bytecodeSegments
    len = matte_array_get_size(bytecodeSegments);
    for(i = 0; i < len; ++i) {
        uint32_t length = matte_array_at(bytecodeSegmentSizes, uint32_t, i);
        matteString_t * name = matte_array_at(bytecodeSegmentNames, matteString_t *, i);
        const uint8_t * data = matte_array_at(bytecodeSegments, uint8_t *, i);
        
        SESBytecodeSegment seg;
        seg.name = matte_string_clone(name);
        seg.length = length;
        seg.data = malloc(length);
        memcpy(seg.data, data, length);
        
        matte_array_push(rom->bytecodeSegments, seg);
    }

    // sub cartridges
    len = matte_array_get_size(subcartridgeROMs);
    for(i = 0; i < len; ++i) {
        uint32_t length = matte_array_at(subcartridgeROMSizes, uint32_t, i);
        matteString_t * name = matte_array_at(subcartridgeNames, matteString_t *, i);
        const uint8_t * data = matte_array_at(subcartridgeROMs, uint8_t *, i);
        
        SESSubCartridge sub;
        sub.name = matte_string_clone(name);
        sub.length = length;
        sub.data = malloc(length);
        memcpy(sub.data, data, length);
        
        matte_array_push(rom->subcartridges, sub);
    }

    return rom;
}




#define PUSHBYTE(__V__) {uint8_t g = __V__; matte_array_push(bytes, g);}
#define PUSHCOPY(__T__, __V__) {__T__ g = __V__; uint32_t n; matte_array_push_n(bytes, (uint8_t*)&g, sizeof(__T__));}
#define PUSHN(__N__, __VP__) {uint32_t n; matte_array_push_n(bytes, __VP__, __N__);}
matteArray_t * ses_rom_pack(const sesROM_t * rom) {




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


    PUSHCOPY(uint32_t, matte_array_get_size(rom->waveforms));
    PUSHCOPY(uint32_t, matte_array_get_size(rom->tiles));
    PUSHCOPY(uint32_t, matte_array_get_size(rom->palettes));
    PUSHCOPY(uint32_t, matte_array_get_size(rom->bytecodeSegments));
    PUSHCOPY(uint32_t, matte_array_get_size(rom->subcartridges));


    uint32_t i, len;
    
    // waveforms
    len = matte_array_get_size(rom->waveforms);
    for(i = 0; i < len; ++i) {
        SESWaveform wav = matte_array_at(rom->waveforms, SESWaveform, i);
    
        uint32_t length = wav.length;
        uint8_t * data = wav.data;

        PUSHCOPY(uint32_t, length);
        PUSHN(length, data);
    }
    
    // tiles 
    len = matte_array_get_size(rom->tiles);
    for(i = 0; i < len; ++i) {
        SESTile tile = matte_array_at(rom->tiles, SESTile, i);
        PUSHN(
            sizeof(SESTile),
            &tile
        );
    };
    

    // palettes
    len = matte_array_get_size(rom->palettes);
    for(i = 0; i < len; ++i) {
        SESPalette palette = matte_array_at(rom->tiles, SESPalette, i);
        PUSHN(
            sizeof(SESPalette),
            &palette
        );
    };


    // bytecodeSegments
    len = matte_array_get_size(rom->bytecodeSegments);
    for(i = 0; i < len; ++i) {
        SESBytecodeSegment seg = matte_array_at(rom->bytecodeSegments, SESBytecodeSegment, i);
    
        uint32_t length = seg.length;
        PUSHCOPY(uint32_t, length);

        matteString_t * name = seg.name;
        uint32_t nameLen = matte_string_get_byte_length(name);
        PUSHCOPY(uint32_t, nameLen);
        
        PUSHN(
            nameLen,
            matte_string_get_byte_data(name)
        );


        PUSHN(
            length,
            seg.data
        );
    }
    
    
    len = matte_array_get_size(rom->subcartridges);
    for(i = 0; i < len; ++i) {
        SESSubCartridge sub = matte_array_at(rom->subcartridges, SESSubCartridge, i);
    
        uint32_t length = sub.length;
        PUSHCOPY(uint32_t, length);

        matteString_t * name = sub.name;
        uint32_t nameLen = matte_string_get_byte_length(name);
        PUSHCOPY(uint32_t, nameLen);
        
        PUSHN(
            nameLen,
            matte_string_get_byte_data(name)
        );


        PUSHN(
            length,
            sub.data
        );
    }
    
    
    return bytes;
    

}





void ses_rom_destroy(sesROM_t * rom) {
    // clear existing ROM
    uint32_t i;
    uint32_t len;
    len = matte_array_get_size(rom->waveforms);
    for(i = 0; i < len; ++i) {
        free(matte_array_at(rom->waveforms, SESWaveform, i).data);
    }
    matte_array_destroy(rom->waveforms);
    matte_array_destroy(rom->tiles);
    matte_array_destroy(rom->palettes);
    len = matte_array_get_size(rom->bytecodeSegments);
    for(i = 0; i < len; ++i) {
        matte_string_destroy(matte_array_at(rom->bytecodeSegments, SESBytecodeSegment, i).name);
        free(matte_array_at(rom->bytecodeSegments, SESBytecodeSegment, i).data);
    }
    matte_array_destroy(rom->bytecodeSegments);

    len = matte_array_get_size(rom->subcartridges);
    for(i = 0; i < len; ++i) {
        matte_string_destroy(matte_array_at(rom->subcartridges, SESSubCartridge, i).name);
        free(matte_array_at(rom->subcartridges, SESSubCartridge, i).data);
    }
    matte_array_destroy(rom->subcartridges);
    
    free(rom);
}



uint32_t ses_rom_get_waveform_count(const sesROM_t * rom) {
    return matte_array_get_size(rom->waveforms);
}

const uint8_t * ses_rom_get_waveform(const sesROM_t * rom, uint32_t id, uint32_t * lengthBytes) {
    *lengthBytes = 0;
    if (id >= ses_rom_get_waveform_count(rom)) return NULL;

    SESWaveform wave = matte_array_at(rom->waveforms, SESWaveform, id);
    *lengthBytes = wave.length;
    return wave.data;
}






uint32_t ses_rom_get_tile_count(const sesROM_t * rom) {
    return matte_array_get_size(rom->tiles);
}

const uint8_t * ses_rom_get_tile(const sesROM_t * rom, uint32_t index, uint32_t * id) {
    if (index >= ses_rom_get_tile_count(rom)) return NULL;
    
    SESTile * tile = &matte_array_at(rom->tiles, SESTile, index);
    *id = tile->id;
    return tile->data;
}



uint32_t ses_rom_get_palette_count(const sesROM_t * rom) {
    return matte_array_get_size(rom->palettes);
}

const float * ses_rom_get_palette(const sesROM_t * rom, uint32_t index, uint32_t * id) {
    if (index >= ses_rom_get_palette_count(rom)) return NULL;
    

    SESPalette * palette = &matte_array_at(rom->tiles, SESPalette, index);
    *id = palette->id;
    return palette->data;
}






uint32_t ses_rom_get_bytecode_segment_count(const sesROM_t * rom) {
    return matte_array_get_size(rom->bytecodeSegments);
}

const uint8_t * ses_rom_get_bytecode_segment(const sesROM_t * rom, uint32_t id, uint32_t * length, matteString_t * name) {
    *length = 0;
    if (id >= ses_rom_get_bytecode_segment_count(rom)) return NULL;

    SESBytecodeSegment seg = matte_array_at(rom->bytecodeSegments, SESBytecodeSegment, id);
    *length = seg.length;
    matte_string_set(name, seg.name);
    return seg.data;
}

uint32_t ses_rom_get_subcartridge_rom_count(const sesROM_t * rom) {
    return matte_array_get_size(rom->subcartridges);
}

const uint8_t * ses_rom_get_subcartridge_rom(const sesROM_t * rom, uint32_t id, uint32_t * length, matteString_t * name) {
    *length = 0;
    if (id >= ses_rom_get_subcartridge_rom_count(rom)) return NULL;

    SESSubCartridge sub = matte_array_at(rom->subcartridges, SESSubCartridge, id);
    *length = sub.length;
    matte_string_set(name, sub.name);
    return sub.data;
}






