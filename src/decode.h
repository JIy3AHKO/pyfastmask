#pragma once

#include <array>
#include <algorithm>
#include <cstring>
#include <vector>
#include <cstdint>
#include "fastmask.h"



inline void get_integer8(const char*& data, uint8_t& value) {
    value = *reinterpret_cast<const uint8_t*>(data);
    data += 1;
}

inline void get_integer16(const char*& data, uint16_t& value) {
    value = *reinterpret_cast<const uint16_t*>(data);
    data += 2;
}

inline void get_integer8_12_12(const char*& data, uint8_t& value1, uint16_t& value2, uint16_t& value3) {
    uint32_t value = *reinterpret_cast<const uint32_t*>(data);
    value1 = value & 0xFF;
    value2 = (value >> 8) & 0xFFF;
    value3 = (value >> 20) & 0xFFF;
    data += 4;
}



inline Header read_header(const char* data) {
    Header header;
    std::memcpy(&header, data, sizeof(Header));

    return header;
}



inline void decode_mask(
    const char* data,
    const Header& header,
    unsigned char * mask
) {

    std::vector<uint8_t> unique_symbols(header.unique_symbols_count);

    for (unsigned int i = 0; i < header.unique_symbols_count; ++i) {
        get_integer8(data, unique_symbols[i]);
    }
    
    uint8_t symbol_id;
    uint16_t count;
    uint16_t line_len;
    uint16_t skip_count;

    // read first line as regular rle
    get_integer16(data, line_len);
    for (size_t i = 0; i < line_len; ++i) {
        get_integer8(data, symbol_id);
        get_integer16(data, count);
        memset(mask, unique_symbols[symbol_id], count);
        mask += count;
    }

    // read the rest of the lines as diffs
    for (uint32_t i = 1; i < header.mask_height; ++i) {
        memcpy(mask, mask - header.mask_width, header.mask_width);

        uint64_t offset = 0;

        get_integer16(data, line_len);
        for (uint32_t j = 0; j < line_len; ++j) {
            get_integer8_12_12(data, symbol_id, skip_count, count);
            offset += skip_count;

            memset(mask + offset, unique_symbols[symbol_id], count);
            offset += count;
        }

        mask += header.mask_width;

    }
}
