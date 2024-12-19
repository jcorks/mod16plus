#include "rom.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


typedef struct {
    uint32_t length;
    uint8_t * data;
} MOD16Waveform;

typedef struct {
    uint32_t id;
    uint8_t data[64];
} MOD16Tile;

typedef struct {
    uint32_t id;
    float data[15];
} MOD16Palette;


typedef struct {
    matteString_t * name;
    uint32_t length;
    uint8_t * data;
} MOD16BytecodeSegment;

typedef struct {
    matteString_t * name;
    uint32_t length;
    uint8_t * data;
} MOD16SubCartridge;




struct mod16ROM_t {
    matteArray_t * waveforms;
    matteArray_t * tiles;
    matteArray_t * palettes;
    matteArray_t * bytecodeSegments;
    matteArray_t * subcartridges;
};



#define MOD16_CHOMP(__T__) *((__T__*)(romBytes+iter)); iter+=sizeof(__T__); if(iter > romLength) goto L_BAD_SIZE;

#define MOD16_CHOMP_LARGE(__N__, __DEST__) {\
  if (iter + (__N__) > romLength) goto L_BAD_SIZE;\
  memcpy(__DEST__, romBytes+iter, __N__);\
  iter+=__N__;\
}
  
  


mod16ROM_t * mod16_rom_unpack(const uint8_t * romBytes, uint32_t romLength, mod16ROM_UnpackError_t * error) {
    mod16ROM_t * rom = calloc(1, sizeof(mod16ROM_t));
    *error = MOD16_ROM_UNPACK_NO_ERROR;

    rom->waveforms = matte_array_create(sizeof(MOD16Waveform));
    rom->tiles = matte_array_create(sizeof(MOD16Tile));
    rom->palettes = matte_array_create(sizeof(MOD16Palette));
    rom->bytecodeSegments = matte_array_create(sizeof(MOD16BytecodeSegment));
    rom->subcartridges = matte_array_create(sizeof(MOD16SubCartridge));


    uint32_t i, len;


    // before starting, check the size.
    if (romLength < 12 + 4*sizeof(uint32_t)) {
        *error = MOD16_ROM_UNPACK_ERROR__TOO_SMALL;
        mod16_rom_destroy(rom);
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
        *error = MOD16_ROM_UNPACK_ERROR__BAD_HEADER;
        mod16_rom_destroy(rom);
        return NULL;
    }
    
    if (romBytes[12] != 1) {
        *error = MOD16_ROM_UNPACK_ERROR__UNSUPPORTED_VERSION;
        mod16_rom_destroy(rom);
        return NULL;
    };
    iter += 13;
        
        
    uint32_t waveformCount        = MOD16_CHOMP(uint32_t);
    uint32_t tileCount            = MOD16_CHOMP(uint32_t);
    uint32_t paletteCount         = MOD16_CHOMP(uint32_t);
    uint32_t bytecodeSegmentCount = MOD16_CHOMP(uint32_t);
    uint32_t subcartridgeCount    = MOD16_CHOMP(uint32_t);
    
    
    
    // waveforms
    for(i = 0; i < waveformCount; ++i) {
        MOD16Waveform wave;
        wave.length = MOD16_CHOMP(uint32_t);
        wave.data = malloc(wave.length);
        
        MOD16_CHOMP_LARGE(wave.length, wave.data);
        matte_array_push(rom->waveforms, wave);
    }
    
    // tiles 
    matte_array_set_size(rom->tiles, tileCount);
    MOD16_CHOMP_LARGE(
        tileCount * sizeof(MOD16Tile),
        matte_array_get_data(rom->tiles)
    );
    

    // palettes
    matte_array_set_size(rom->palettes, paletteCount);
    MOD16_CHOMP_LARGE(
        paletteCount * sizeof(MOD16Palette),
        matte_array_get_data(rom->palettes)
    );



    // bytecodeSegments
    for(i = 0; i < bytecodeSegmentCount; ++i) {
        MOD16BytecodeSegment seg;
        seg.length = MOD16_CHOMP(uint32_t);        
        uint32_t nameLenBytes = MOD16_CHOMP(uint32_t);
        char * name = malloc(nameLenBytes+1);
        MOD16_CHOMP_LARGE(nameLenBytes, name);
        name[nameLenBytes] = 0;
        seg.name = matte_string_create_from_c_str("%s", name);
        free(name);
        seg.data = malloc(seg.length);
        MOD16_CHOMP_LARGE(seg.length, seg.data);
        matte_array_push(rom->bytecodeSegments, seg);
    }

    // subcartridges
    for(i = 0; i < subcartridgeCount; ++i) {
        MOD16SubCartridge sub;
        sub.length = MOD16_CHOMP(uint32_t);        
        uint32_t nameLenBytes = MOD16_CHOMP(uint32_t);
        char * name = malloc(nameLenBytes+1);
        MOD16_CHOMP_LARGE(nameLenBytes, name);
        name[nameLenBytes] = 0;
        sub.name = matte_string_create_from_c_str("%s", name);
        free(name);
        sub.data = malloc(sub.length);
        MOD16_CHOMP_LARGE(sub.length, sub.data);
        matte_array_push(rom->subcartridges, sub);
    }

    
    
    return rom;         
    

    // chomp can send
  L_BAD_SIZE:
    mod16_rom_destroy(rom);
    *error = MOD16_ROM_UNPACK_ERROR__SIZE_MISMATCH;
    return NULL;
  
    
}

mod16ROM_t * mod16_rom_create(

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
    matteArray_t * tiles, // uint8_t *, see mod16_rom_get_tile


    // Pre-computed palettes and their associated IDs. 
    // Like tiles, palettes can be made during runtime, so 
    // these are just the initial set.
    // Palettes are 15 floating point values.
    matteArray_t * paletteIDs, // uint32_t
    matteArray_t * palettes, // uint8_t *, see mod16_rom_get_palette
    
    // Pre-compiled sources and their associated names.    
    // Each bytecode segment is expected to be compiled Matte 
    // bytecode.
    matteArray_t * bytecodeSegmentNames, // matteString_t *
    matteArray_t * bytecodeSegmentSizes, // uint32_t
    matteArray_t * bytecodeSegments, // uint8_t *
    
    // Pre-compiled MOD16 cartridge ROMs to be accessible as 
    // sub-cartridges. Each subcartridge is named, and each 
    // rom is assumed to have been made with mod16_rom_pack() 
    matteArray_t * subcartridgeNames, // matteString_t *
    matteArray_t * subcartridgeROMSizes, // uint32_t 
    matteArray_t * subcartridgeROMs // uint8_t *
) {
    mod16ROM_t * rom = calloc(1, sizeof(mod16ROM_t));

    rom->waveforms = matte_array_create(sizeof(MOD16Waveform));
    rom->tiles = matte_array_create(sizeof(MOD16Tile));
    rom->palettes = matte_array_create(sizeof(MOD16Palette));
    rom->bytecodeSegments = matte_array_create(sizeof(MOD16BytecodeSegment));
    rom->subcartridges = matte_array_create(sizeof(MOD16SubCartridge));


    uint32_t i, len;
    
    // waveforms
    len = matte_array_get_size(waveforms);
    for(i = 0; i < len; ++i) {
        uint32_t length = matte_array_at(waveformSizes, uint32_t, i);
        uint8_t * data = matte_array_at(waveforms, uint8_t *, i);

        MOD16Waveform wav = {};
        wav.length = length;
        wav.data = malloc(length);
        memcpy(wav.data, data, wav.length);
        
        matte_array_push(rom->subcartridges, wav);
    }
    
    // tiles 
    len = matte_array_get_size(tiles);
    for(i = 0; i < len; ++i) {
        MOD16Tile tile;
        tile.id = matte_array_at(tileIDs, uint32_t, i);
        memcpy(tile.data, &matte_array_at(tiles, uint8_t, i*64), 64);

        matte_array_push(rom->tiles, tile);
    };
    

    // palettes
    len = matte_array_get_size(palettes);
    for(i = 0; i < len; ++i) {
        MOD16Palette palette;
        palette.id = matte_array_at(paletteIDs, uint32_t, i);
        memcpy(palette.data, &matte_array_at(palettes, float, i*15), 15*sizeof(float));

        matte_array_push(rom->palettes, palette);
    };


    // bytecodeSegments
    len = matte_array_get_size(bytecodeSegments);
    for(i = 0; i < len; ++i) {
        uint32_t length = matte_array_at(bytecodeSegmentSizes, uint32_t, i);
        matteString_t * name = matte_array_at(bytecodeSegmentNames, matteString_t *, i);
        const uint8_t * data = matte_array_at(bytecodeSegments, uint8_t *, i);
        
        MOD16BytecodeSegment seg;
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
        
        MOD16SubCartridge sub;
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
matteArray_t * mod16_rom_pack(const mod16ROM_t * rom) {




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
        MOD16Waveform wav = matte_array_at(rom->waveforms, MOD16Waveform, i);
    
        uint32_t length = wav.length;
        uint8_t * data = wav.data;

        PUSHCOPY(uint32_t, length);
        PUSHN(length, data);
    }
    
    // tiles 
    len = matte_array_get_size(rom->tiles);
    for(i = 0; i < len; ++i) {
        MOD16Tile tile = matte_array_at(rom->tiles, MOD16Tile, i);
        PUSHN(
            sizeof(MOD16Tile),
            &tile
        );
    };
    

    // palettes
    len = matte_array_get_size(rom->palettes);
    for(i = 0; i < len; ++i) {
        MOD16Palette palette = matte_array_at(rom->palettes, MOD16Palette, i);
        PUSHN(
            sizeof(MOD16Palette),
            &palette
        );
    };


    // bytecodeSegments
    len = matte_array_get_size(rom->bytecodeSegments);
    for(i = 0; i < len; ++i) {
        MOD16BytecodeSegment seg = matte_array_at(rom->bytecodeSegments, MOD16BytecodeSegment, i);
    
        uint32_t length = seg.length;
        PUSHCOPY(uint32_t, length);

        matteString_t * name = seg.name;
        uint32_t nameLen = matte_string_get_utf8_length(name);
        PUSHCOPY(uint32_t, nameLen);
        
        PUSHN(
            nameLen,
            matte_string_get_utf8_data(name)
        );


        PUSHN(
            length,
            seg.data
        );
    }
    
    
    len = matte_array_get_size(rom->subcartridges);
    for(i = 0; i < len; ++i) {
        MOD16SubCartridge sub = matte_array_at(rom->subcartridges, MOD16SubCartridge, i);
    
        uint32_t length = sub.length;
        PUSHCOPY(uint32_t, length);

        matteString_t * name = sub.name;
        uint32_t nameLen = matte_string_get_utf8_length(name);
        PUSHCOPY(uint32_t, nameLen);
        
        PUSHN(
            nameLen,
            matte_string_get_utf8_data(name)
        );


        PUSHN(
            length,
            sub.data
        );
    }
    
    
    return bytes;
    

}





void mod16_rom_destroy(mod16ROM_t * rom) {
    // clear existing ROM
    uint32_t i;
    uint32_t len;
    len = matte_array_get_size(rom->waveforms);
    for(i = 0; i < len; ++i) {
        free(matte_array_at(rom->waveforms, MOD16Waveform, i).data);
    }
    matte_array_destroy(rom->waveforms);
    matte_array_destroy(rom->tiles);
    matte_array_destroy(rom->palettes);
    len = matte_array_get_size(rom->bytecodeSegments);
    for(i = 0; i < len; ++i) {
        matte_string_destroy(matte_array_at(rom->bytecodeSegments, MOD16BytecodeSegment, i).name);
        free(matte_array_at(rom->bytecodeSegments, MOD16BytecodeSegment, i).data);
    }
    matte_array_destroy(rom->bytecodeSegments);

    len = matte_array_get_size(rom->subcartridges);
    for(i = 0; i < len; ++i) {
        matte_string_destroy(matte_array_at(rom->subcartridges, MOD16SubCartridge, i).name);
        free(matte_array_at(rom->subcartridges, MOD16SubCartridge, i).data);
    }
    matte_array_destroy(rom->subcartridges);
    
    free(rom);
}



uint32_t mod16_rom_get_waveform_count(const mod16ROM_t * rom) {
    return matte_array_get_size(rom->waveforms);
}

const uint8_t * mod16_rom_get_waveform(const mod16ROM_t * rom, uint32_t id, uint32_t * lengthBytes) {
    *lengthBytes = 0;
    if (id >= mod16_rom_get_waveform_count(rom)) return NULL;

    MOD16Waveform wave = matte_array_at(rom->waveforms, MOD16Waveform, id);
    *lengthBytes = wave.length;
    return wave.data;
}






uint32_t mod16_rom_get_tile_count(const mod16ROM_t * rom) {
    return matte_array_get_size(rom->tiles);
}

const uint8_t * mod16_rom_get_tile(const mod16ROM_t * rom, uint32_t index, uint32_t * id) {
    if (index >= mod16_rom_get_tile_count(rom)) return NULL;
    
    MOD16Tile * tile = &matte_array_at(rom->tiles, MOD16Tile, index);
    *id = tile->id;
    return tile->data;
}



uint32_t mod16_rom_get_palette_count(const mod16ROM_t * rom) {
    return matte_array_get_size(rom->palettes);
}

const float * mod16_rom_get_palette(const mod16ROM_t * rom, uint32_t index, uint32_t * id) {
    if (index >= mod16_rom_get_palette_count(rom)) return NULL;
    

    MOD16Palette * palette = &matte_array_at(rom->palettes, MOD16Palette, index);
    *id = palette->id;
    return palette->data;
}






uint32_t mod16_rom_get_bytecode_segment_count(const mod16ROM_t * rom) {
    return matte_array_get_size(rom->bytecodeSegments);
}

const uint8_t * mod16_rom_get_bytecode_segment(const mod16ROM_t * rom, uint32_t id, uint32_t * length, matteString_t * name) {
    *length = 0;
    if (id >= mod16_rom_get_bytecode_segment_count(rom)) return NULL;

    MOD16BytecodeSegment seg = matte_array_at(rom->bytecodeSegments, MOD16BytecodeSegment, id);
    *length = seg.length;
    matte_string_set(name, seg.name);
    return seg.data;
}

uint32_t mod16_rom_get_subcartridge_rom_count(const mod16ROM_t * rom) {
    return matte_array_get_size(rom->subcartridges);
}

const uint8_t * mod16_rom_get_subcartridge_rom(const mod16ROM_t * rom, uint32_t id, uint32_t * length, matteString_t * name) {
    *length = 0;
    if (id >= mod16_rom_get_subcartridge_rom_count(rom)) return NULL;

    MOD16SubCartridge sub = matte_array_at(rom->subcartridges, MOD16SubCartridge, id);
    *length = sub.length;
    matte_string_set(name, sub.name);
    return sub.data;
}






