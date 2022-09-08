#ifndef H_SES_DUMP_INCLUDED
#define H_SES_DUMP_INCLUDED

#include <stdint.h>

// Given a file path, returns a dump of the file 
// into a byte array. len is populated with its length.
// Exits the program on failure with a message to stdout.
void * dump_bytes(const char * filename, uint32_t * len);



// Given a path and a byte array, dumps the bytes to a file.
// Success is returned.
int dump_file(const char * filename, void * bytes, uint32_t len);

#endif
