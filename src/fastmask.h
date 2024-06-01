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


/**
 * @brief Represents the header structure of a FastMask file.
 *
 * The Header struct contains information about the file format and dimensions of the mask.
 */
struct Header {
    uint32_t magic;                 /**< The magic number of the file. */
    uint8_t version;                /**< The version of the file format. */
    uint8_t symbol_bit_width;       /**< The bit width of each symbol. */
    uint8_t count_bit_width;        /**< The bit width of the symbol count. */
    uint8_t line_count_bit_width;   /**< The bit width of the count of groups of symbols in a line. */
    uint32_t unique_symbols_count;  /**< The number of unique symbols in the file. */
    uint32_t mask_height;           /**< The height of the mask. */
    uint32_t mask_width;            /**< The width of the mask. */
};
