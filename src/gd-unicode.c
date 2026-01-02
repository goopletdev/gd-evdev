#include "gd-unicode.h"

const char *gd_hex_pairs[256] = {
    "00", "01", "02", "03", "04", "05", "06", "07",
    "08", "09", "0a", "0b", "0c", "0d", "0e", "0f",
    "10", "11", "12", "13", "14", "15", "16", "17",
    "18", "19", "1a", "1b", "1c", "1d", "1e", "1f",
    "20", "21", "22", "23", "24", "25", "26", "27",
    "28", "29", "2a", "2b", "2c", "2d", "2e", "2f",
    "30", "31", "32", "33", "34", "35", "36", "37",
    "38", "39", "3a", "3b", "3c", "3d", "3e", "3f",
    "40", "41", "42", "43", "44", "45", "46", "47",
    "48", "49", "4a", "4b", "4c", "4d", "4e", "4f",
    "50", "51", "52", "53", "54", "55", "56", "57",
    "58", "59", "5a", "5b", "5c", "5d", "5e", "5f",
    "60", "61", "62", "63", "64", "65", "66", "67",
    "68", "69", "6a", "6b", "6c", "6d", "6e", "6f",
    "70", "71", "72", "73", "74", "75", "76", "77",
    "78", "79", "7a", "7b", "7c", "7d", "7e", "7f",
    "80", "81", "82", "83", "84", "85", "86", "87",
    "88", "89", "8a", "8b", "8c", "8d", "8e", "8f",
    "90", "91", "92", "93", "94", "95", "96", "97",
    "98", "99", "9a", "9b", "9c", "9d", "9e", "9f",
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
    "a8", "a9", "aa", "ab", "ac", "ad", "ae", "af",
    "b0", "b1", "b2", "b3", "b4", "b5", "b6", "b7",
    "b8", "b9", "ba", "bb", "bc", "bd", "be", "bf",
    "c0", "c1", "c2", "c3", "c4", "c5", "c6", "c7",
    "c8", "c9", "ca", "cb", "cc", "cd", "ce", "cf",
    "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
    "d8", "d9", "da", "db", "dc", "dd", "de", "df",
    "e0", "e1", "e2", "e3", "e4", "e5", "e6", "e7",
    "e8", "e9", "ea", "eb", "ec", "ed", "ee", "ef",
    "f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7",
    "f8", "f9", "fa", "fb", "fc", "fd", "fe", "ff",
};

int utf8_high_leading_bits(unsigned char c) {
    int high_leading_bits = 0;
    for (unsigned char mask = 0x80; mask & c; mask >>=1) {
        high_leading_bits++;
    }
    return high_leading_bits;
}

int utf8_codepoint_from_str(const char *buffer, size_t s, size_t pos, utf8_codepoint *codepoint) {
    // check if pos is in buffer range
    if (pos > s - 1) return -4;

    // get number of leading high bits
    unsigned char c = buffer[pos];
    int high_leading_bits = utf8_high_leading_bits(c);

    // check for errors
    switch (high_leading_bits) {
        case 0:
            // ascii character
            *codepoint = (utf8_codepoint)c;
            return 1;
        case 1:
            // continuation character
            return -1;
        case 2:
        case 3:
        case 4:
            break;
        default:
            // implied more than 4 bytes
            return -2;
    }

    // shift codepoint bits into single utf8_codepointeger
    unsigned char bitmask = ((1U << (CHAR_BIT - high_leading_bits)) - 1);
    utf8_codepoint codepoint_value = (utf8_codepoint)(c & bitmask);
    // set bitmask for getting continuation character value
    bitmask = 0x3FU;
    size_t max_pos = pos + high_leading_bits;
    // check whether sufficient bytes available in buffer
    if (max_pos > s) return -3;
    for (pos++; pos < max_pos; pos++) {
        if (utf8_high_leading_bits(buffer[pos]) != 1) return -5;
        codepoint_value <<= 6;
        codepoint_value |= (bitmask & buffer[pos]);
    }
    
    // set number of bytes and return codepoint
    *codepoint = codepoint_value;
    return high_leading_bits;
}

int utf8_str_from_codepoint(char *buffer, size_t s, size_t pos, utf8_codepoint codepoint) {
    int bytes = 0;
    unsigned char validation_buffer[4];
    if (codepoint < 0x80U) {
        // is ascii
        bytes = 1;
        validation_buffer[0] = (unsigned char)codepoint; 
    } else if (codepoint < (1U << 11)) {
        bytes = 2;
        validation_buffer[0] = (codepoint >> 6) | 0xC0U;
        validation_buffer[1] = (codepoint & 0x3FU) | 0x80U;
    } else if (codepoint < (1U << 16)) {
        bytes = 3;
        validation_buffer[0] = (codepoint >> 12) | 0xE0U;
        validation_buffer[1] = ((codepoint >> 6) & 0x3FU) | 0x80U;
        validation_buffer[2] = (codepoint & 0x3FU) | 0x80U;
    } else if (codepoint < (1U << 21)) {
        bytes = 4;
        validation_buffer[0] = (codepoint >> 18) | 0xF0U;
        validation_buffer[1] = ((codepoint >> 12) & 0x3FU) | 0x80U;
        validation_buffer[2] = ((codepoint >> 6) & 0x3FU) | 0x80U;
        validation_buffer[3] = (codepoint & 0x3FU) | 0x80U;
    } else {
        // disallowed unicode codepoint 
        return -1;
    }

    // check buffer size 
    size_t max_pos = pos + bytes;
    if (max_pos > s) {
        // not enough space in buffer
        return -2;
    }

    unsigned char *vbuffer_ptr = validation_buffer;
    for (size_t i = pos; i < max_pos; i++) {
        buffer[i] = *vbuffer_ptr++;
    }

    return bytes;
}

