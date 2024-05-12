#pragma once
#include <cstdint>

#define buffer_t uint64_t
#define buffer_t_bits 64

const uint8_t VERSION_BYTE = 0x01;
const uint32_t MAGIC_BYTE = (
      ((uint32_t)'p')
    | ((uint32_t)'f') << 8
    | ((uint32_t)'m') << 16
    | ((uint32_t)'f') << 24
);


struct Header {
    uint32_t magic;
    uint8_t version;
    uint8_t symbol_bit_width;
    uint8_t count_bit_width;
    uint8_t line_count_bit_width;
    uint32_t unique_symbols_count;
    uint32_t mask_height;
    uint32_t mask_width;
};
