#ifndef H_SES_ROM_INCLUDED
#define H_SES_ROM_INCLUDED


#include "matte/src/matte_string.h"
#include "matte/src/matte_array.h"



typedef struct sesROM_t sesROM_t;



// An error code is returned.
typedef enum {
    // No error while unpacking.
    SES_ROM_UNPACK_NO_ERROR,
    // the ROM is too small to be valid.
    //
    SES_ROM_UNPACK_ERROR__TOO_SMALL,
    // the header is incorrect. This can happen if the very start
    // of the file is corrupted, or the file is not a ROM file.
    SES_ROM_UNPACK_ERROR__BAD_HEADER,
    
    // the ROM is of an unsupported version.
    SES_ROM_UNPACK_ERROR__UNSUPPORTED_VERSION,
    
    // the ROM data has an invalid size of some kind,
    // likely indicating a corrupted ROM.
    SES_ROM_UNPACK_ERROR__SIZE_MISMATCH,
} sesROM_UnpackError_t;


// Sets and unpacks the resources for a new rom
//
// NOTE: within the Matte context, the rom controls what 
// additional scripts are visible, as ROMs contain pre-compiled 
// bytecode blobs of Matte logic that the games themselves 
// import.
// 
sesROM_t * ses_rom_unpack(const uint8_t * romBytes, uint32_t romLength, sesROM_UnpackError_t * error);



// Creates a new rom based on the given inputs.
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
);


// Packs the rom into bytes. This can be used 
// to represent subcartridges or to unpack in a different instance 
// of SES. The array returned is in bytes.
matteArray_t * ses_rom_pack(const sesROM_t *);



// Destroys the rom and any associated data.
void ses_rom_destroy(sesROM_t *);




// Gets the number of waveforms within the ROM.
uint32_t ses_rom_get_waveform_count(const sesROM_t *);

// Gets the IDd waveform from the rom.
// If no waveform exists, NULL is returned/
const uint8_t * ses_rom_get_waveform(const sesROM_t *, uint32_t id, uint32_t * lengthBytes);



// Gets the number of pre-defined tiles within the 
// ROM. 
uint32_t ses_rom_get_tile_count(const sesROM_t *);

// Returns the raw tile data for the given tile.
// If no such tile exists within the rom, NULL is returned.
//
// tile data is written in contiguous data segments, left to right, 
// top to bottom, each each pixel as one byte.
//
// 0 -> transparent
// 1 -> background color 
// 2 -> mid-background color
// 2 -> mid-front color 
// 3 -> front color 
//
// Each tile in the SES is always 8x8, so this block is 
// always 64 bytes.
const uint8_t * ses_rom_get_tile(const sesROM_t *, uint32_t index, uint32_t * id);




// Gets the number of pre-defined palettes within the
// ROM.
uint32_t ses_rom_get_palette_count(const sesROM_t *);

// Gets the palette in the ROM. Returns null if none at that index within the rom.
// Palettes are always packages in RGB, one byte for each component.
// Palette color orders are always: background, mid-background, mid-front, and front
// in order of bytes.
// Palettes returned are always 12 floats.
const float * ses_rom_get_palette(const sesROM_t *, uint32_t index, uint32_t * id);





// Gets the number of bytecode segments in the 
// rom. SES has very few filesystem features; all imported code 
// during runtime is pulled from the ROM. 
uint32_t ses_rom_get_bytecode_segment_count(const sesROM_t *);

// Retrives the bytecode segment at the given index.
// The raw bytes are returned. Length is populated with the number 
// of bytes in the returned segment.
// Name should be an allocated string. It will be 
// updated with the name of the segment.
const uint8_t * ses_rom_get_bytecode_segment(const sesROM_t *, uint32_t i, uint32_t * length, matteString_t * name);



// Gets the number of subcartridges in the 
// rom.  
uint32_t ses_rom_get_subcartridge_rom_count(const sesROM_t *);

// Retrieves the subcartridge at the given index.
// The raw bytes are returned. Length is populated with the number 
// of bytes in the returned segment.
// Name should be an allocated string. It will be 
// updated with the name of the subcartridge.
const uint8_t * ses_rom_get_subcartridge_rom(const sesROM_t *, uint32_t i, uint32_t * length, matteString_t * name);





#endif
