#pragma once

#include <array>
#include <algorithm>
#include <cstring>
#include <vector>
#include <cstdint>
#include "fastmask.h"


#define buffer_t uint64_t
#define buffer_t_bits 64

const std::array<buffer_t, buffer_t_bits> bitmasks = []() {
    std::array<buffer_t, buffer_t_bits> bitmasks;
    for (char i = 0; i < buffer_t_bits; i++) {
        bitmasks[i] = (1ULL << i) - 1ULL;
    }

    return bitmasks;
}();

inline buffer_t get_integer(int bits, const buffer_t*& data, int& bit_offset) {
    buffer_t value = (*data >> bit_offset) & (bitmasks[bits]);

    if (bit_offset + bits >= buffer_t_bits) {
        data++;
        value |= (*data & (bitmasks[bit_offset + bits - buffer_t_bits])) << (buffer_t_bits - bit_offset);
        bit_offset = bit_offset + bits - buffer_t_bits;
    } else {
        bit_offset += bits;
    }

    return value;
}


inline Header read_header(const char* data) {
    Header header;
    std::memcpy(&header, data, sizeof(Header));

    return header;
}



inline void decode_mask(
    const char* encdoded_data,
    const Header& header,
    unsigned char * mask
) {
    const buffer_t* data = reinterpret_cast<const buffer_t*>(encdoded_data);

    int bit_offset = 0;
    std::vector<int> unique_symbols(header.unique_symbols_count);

    for (unsigned int i = 0; i < header.unique_symbols_count; ++i) {
        unique_symbols[i] = get_integer(8, data, bit_offset);
    }
    
    buffer_t symbol_id;
    buffer_t count;
    buffer_t line_len;
    buffer_t skip_count;

    // read first line as regular rle
    line_len = get_integer(header.line_count_bit_width, data, bit_offset);
    for (size_t i = 0; i < line_len; ++i) {
        symbol_id = get_integer(header.symbol_bit_width, data, bit_offset);
        count = get_integer(header.count_bit_width, data, bit_offset);
        std::memset(mask, unique_symbols[symbol_id], count);
        mask += count;
    }

    // read the rest of the lines as diffs
    for (uint32_t i = 1; i < header.mask_height; ++i) {
        std::memcpy(mask, mask - header.mask_width, header.mask_width);

        buffer_t offset = 0;

        line_len = get_integer(header.line_count_bit_width, data, bit_offset);
        for (uint32_t j = 0; j < line_len; ++j) {
            skip_count = get_integer(header.count_bit_width, data, bit_offset);
            offset += skip_count;
            symbol_id = get_integer(header.symbol_bit_width, data, bit_offset);
            count = get_integer(header.count_bit_width, data, bit_offset);
            memset(mask + offset, unique_symbols[symbol_id], count);
            offset += count;
        }

        mask += header.mask_width;

    }
}
