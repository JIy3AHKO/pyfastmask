#pragma once


const unsigned char VERSION_BYTE = 0x01;
const unsigned int MAGIC_BYTE = (
      ((unsigned int)'p')
    | ((unsigned int)'f') << 8
    | ((unsigned int)'m') << 16
    | ((unsigned int)'f') << 24
);


struct Header {
    unsigned int magic;
    unsigned char version;
    unsigned char symbol_bit_width;
    unsigned char count_bit_width;
    unsigned int unique_symbols_count;
    unsigned int intervals;
    unsigned int mask_height;
    unsigned int mask_width;
};
