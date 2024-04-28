#pragma once

#include <array>
#include <cstring>
#include "fastmask.h"

const std::array<unsigned long long, 64> bitmasks = []() {
    std::array<unsigned long long, 64> bitmasks;
    for (char i = 0; i < 64; i++) {
        bitmasks[i] = (1ULL << i) - 1ULL;
    }

    return bitmasks;
}();

template <typename T>
inline void get_integer(char bits, unsigned long long*& data, unsigned char& current_bit_left, T& value) {
    // assume that bits is less than 64
    if (current_bit_left < bits) {
        value = data[0] & bitmasks[current_bit_left];
        bits -= current_bit_left;
        value |= (data[1] & bitmasks[bits]) << current_bit_left;
        current_bit_left = 64 - bits;
        data[1] >>= bits;
        data++;
    } else {
        value = data[0] & bitmasks[bits];
        current_bit_left -= bits;
        data[0] >>= bits;
    }
}


inline Header read_header(const char* data) {
    Header header;
    memcpy(&header, data, sizeof(Header));

    return header;
}



inline void decode_mask(
    unsigned long long* data, 
    const Header& header,
    unsigned char * mask
) {
    unsigned char current_bit_left = 64;
    unsigned char unique_symbols[header.unique_symbols_count];

    for (unsigned int i = 0; i < header.unique_symbols_count; ++i) {
        get_integer<unsigned char>(8, data, current_bit_left, unique_symbols[i]);
    }

    memset(mask, unique_symbols[0], header.mask_height * header.mask_width);

    
    unsigned char symbol_id;
    size_t count;

    for (unsigned int i = 0; i < header.intervals; ++i) {
        get_integer<unsigned char>(header.symbol_bit_width, data, current_bit_left, symbol_id);
        get_integer<size_t>(header.count_bit_width, data, current_bit_left, count);

        if (symbol_id != 0) {
            memset(mask, unique_symbols[symbol_id], count);
        }
        mask += count;
    }
}
