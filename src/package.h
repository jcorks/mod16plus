#ifndef H_SES_PACKAGE_INCLUDED
#define H_SES_PACKAGE_INCLUDED


// Creates a rom.ses based on the files 
// within the given directory.
//
// The directory is expected to contain the following files:
// SOURCES, TILES, PALETTES, BACKGROUNDS, WAVEFORMS
//
// These act as recipe files and should contain paths / names to 
// files relevant to each resource to be packed into the 
// resultant rom.
int ses_package(const char * dir);

#endif
