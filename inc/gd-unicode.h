#ifndef GD_UNICODE_H
#define GD_UNICODE_H

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef unsigned int utf8_codepoint;

/**
 * sets uint at codepoint ptr to codepoint, and returns number of bytes read,
 * or returns negative value for parsing error:
 *  -1: char at pos is a continuation character
 *  -2: char at pos implies more than 4 bytes
 *  -3: insufficient continuation characters in buffer
 *  -4: pos out of buffer's given range
 *  -5: encountered early non-continuation character
 */
int utf8_codepoint_from_str(
        const char *buffer,
        size_t buffer_size,
        size_t pos,
        utf8_codepoint *codepoint);

/**
 * returns number of high leading bits
 */
int utf8_high_leading_bits(unsigned char c);

int utf8_str_from_codepoint(char *buffer,
        size_t s, size_t pos,
        utf8_codepoint codepoint);

extern const char *gd_hex_pairs[256]; 

#endif // GD_UNICODE_H
