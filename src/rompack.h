#ifndef H_SES_ROMPACK_INCLUDED
#define H_SES_ROMPACK_INCLUDED


#include "matte/src/matte_string.h"
#include "matte/src/matte_array.h"

// Sets and unpacks the resources for the current ROM.
//
// NOTE: within the Matte context, the rom controls what 
// additional scripts are visible, as ROMs contain pre-compiled 
// bytecode blobs of Matte logic that the games themselves 
// import.
// 
// An error code is returned.
typedef enum {
    // the ROM is too small to be valid.
    //
    SES_UNPACK_ERROR__TOO_SMALL = 1,
    // the header is incorrect. This can happen if the very start
    // of the file is corrupted, or the file is not a ROM file.
    SES_UNPACK_ERROR__BAD_HEADER,
    
    // the ROM is of an unsupported version.
    SES_UNPACK_ERROR__UNSUPPORTED_VERSION,
    
    // the ROM data has an invalid size of some kind,
    // likely indicating a corrupted ROM.
    SES_UNPACK_ERROR__SIZE_MISMATCH,
} SESUnpackError_t;

SESUnpackError_t ses_unpack_rom(const uint8_t * romBytes, uint32_t romLength);




uint8_t * ses_pack_rom(
    matteArray_t * waveformSizes, // uint32_t
    matteArray_t * waveforms, // uint8_t *     
    
    matteArray_t * tiles, // uint8_t *, see ses_rom_get_tile
    matteArray_t * palettes, // uint8_t *, see ses_rom_get_palette
    matteArray_t * backgrounds, // uint32_t *, see ses_rom_get_background
    
    
    matteArray_t * bytecodeSegmentNames, // matteString_t *
    matteArray_t * bytecodeSegmentSizes, // uint32_t
    matteArray_t * bytecodeSegments // uint8_t *
);





// Gets the nu,ber of waveforms within the ROM.
uint32_t ses_rom_get_waveform_count();

// Gets the IDd waveform from the rom.
// If no waveform exists, NULL is returned/
const uint8_t * ses_rom_get_waveform(uint32_t id, uint32_t * lengthBytes);



// Gets the number of pre-defined tiles within the 
// ROM. 
uint32_t ses_rom_get_tile_count();

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
const uint8_t * ses_rom_get_tile(uint32_t id);




// Gets the number of pre-defined palettes within the
// ROM.
uint32_t ses_rom_get_palette_count();

// Gets the palette in the ROM. Returns null if none at that index within the rom.
// Palettes are always packages in RGB, one byte for each component.
// Palette color orders are always: background, mid-background, mid-front, and front
// in order of bytes.
// Palettes returned are always 12 bytes.
const uint8_t * ses_rom_get_palette(uint32_t i);



// Gets the number of pre-defined backgrounds.
// Background data contains only the pre-ordained arrangement 
// of tiles. All other information is provided 
// during runtime.
uint32_t ses_rom_get_background_count();

// Gets the background at the given index.
// Backgrounds are arrays of tile indexes, from left to right, top to bottom 
// in a contiguous order. Backgrounds are always 8x8 tiles.
const uint32_t * ses_rom_get_background(uint32_t i);





// Gets the number of bytecode segments in the 
// rom. SES has very few filesystem features; all imported code 
// during runtime is pulled from the ROM. 
uint32_t ses_rom_get_bytecode_segment_count();

// Retrives the bytecode segment at the given index.
// The raw bytes are returned. Length is populated with the number 
// of bytes in the returned segment.
// Name should be an allocated string. It will be 
// updated with the name of the segment.
const uint8_t * ses_rom_get_bytecode_segment(uint32_t i, uint32_t * length, matteString_t * name);



#endif
