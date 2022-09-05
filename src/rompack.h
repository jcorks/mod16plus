#ifndef H_SES_ROMPACK_INCLUDED
#define H_SES_ROMPACK_INCLUDED

// Sets and unpacks the resources for the current ROM.
//
// NOTE: within the Matte context, the rom controls what 
// additional scripts are visible, as ROMs contain pre-compiled 
// bytecode blobs of Matte logic that the games themselves 
// import.
void ses_unpack_rom(matte_t *, const uint8_t * romBytes, uint32_t romLength);


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



#endif
