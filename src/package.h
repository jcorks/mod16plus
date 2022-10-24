#ifndef H_SES_PACKAGE_INCLUDED
#define H_SES_PACKAGE_INCLUDED

typedef struct matteVM_t matteVM_t;
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



// adds the external functions required for 
// development features.
void ses_package_bind_natives(matteVM_t * vm);


#endif
