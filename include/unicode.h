// unicode.h
#ifndef UNICODE_H
#define UNICODE_H

#include <exec/types.h>

// Structure for character mapping
struct CharMapping {
   const unsigned char bytes[4];  // UTF-8 byte sequence 
   unsigned char ascii;           // ASCII replacement
   int len;                      // Length of byte sequence
};

// Main conversion function
char *convertToASCII(const unsigned char *input);

#endif /* UNICODE_H */