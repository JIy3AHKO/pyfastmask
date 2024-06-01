#pragma once
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <tuple>
#include <string>
#include <stdexcept>
#include <memory>
#include "fastmask.h"

const uint32_t SAME_AS_PREV = (uint32_t)-1;


/**
 * @class BitWriter
 * @brief A class for writing bits to a byte stream.
 *
 * The BitWriter class provides functionality to write bits to a byte stream. It allows adding integers of a specified
 * number of bits to the stream and retrieves the resulting byte stream.
 */
class BitWriter {
private:
    char running_byte = 0; // byte that is being built
    int running_bits = 0; // number of bits in the running byte

public:
    BitWriter() {}
    std::vector<unsigned char> data;

    /**
     * Adds an integer value to the data stream.
     *
     * @param value The integer value to be added.
     * @param value_bits The number of bits to represent the value.
     *
     * @throws std::invalid_argument If value_bits is greater than 64 or if the value does not fit in the specified number of bits.
     */
    void add_integer(uint64_t value, int value_bits) {
        if (value_bits > 64) {
            std::string message = "value_bits must be less than or equal to 64, got " + std::to_string(value_bits);
            throw std::invalid_argument(message);
        }

        if (value >= (1ULL << value_bits)) {
            std::string message = "Value " + std::to_string(value) + " does not fit in " + std::to_string(value_bits) + " bits.";
            throw std::invalid_argument(message);
        }

        while (value_bits > 0) {
            int bits_to_write = std::min(8 - running_bits, value_bits);
            running_byte |= (value & ((1 << bits_to_write) - 1)) << running_bits;
            value >>= bits_to_write;
            value_bits -= bits_to_write;
            running_bits += bits_to_write;

            if (running_bits == 8) {
                data.push_back(running_byte);
                running_byte = 0;
                running_bits = 0;
            }
        }
    }

    /**
     * @brief Retrieves the encoded data.
     *
     * This function returns the encoded data as a vector of unsigned characters.
     * If there are any remaining bits in the running_byte, it is appended to the data vector.
     *
     * @return The encoded data as a vector of unsigned characters.
     */
    std::vector<unsigned char> get_data() {
        if (running_bits > 0) {
            data.push_back(running_byte);
        }
        return data;
    }

};


uint8_t get_bit_width(uint32_t value) {
    uint8_t bit_width = 0;
    while (value > 0) {
        value >>= 1;
        bit_width++;
    }
    return bit_width;
}

class BaseRLELine {
public:
    std::vector<uint32_t> symbols;
    std::vector<uint32_t> counts;

    virtual ~BaseRLELine() {}

    void add(uint32_t symbol, uint32_t count) {
        symbols.push_back(symbol);
        counts.push_back(count);
    }

    virtual void encode(
        BitWriter& bits,
        std::map<uint32_t, int>& symbol_to_index,
        const Header& header
    ) = 0;
};


class PlainRLELine : public BaseRLELine {
public:
    void build(unsigned char * line, size_t length) {
        uint32_t prev = line[0];
        uint32_t count = 1;

        for (size_t i = 1; i < length; i++) {
            if ((uint32_t)line[i] == prev) {
                count++;
            } else {
                add(prev, count);
                prev = line[i];
                count = 1;
            }
        }

        add(prev, count);
    }

    void encode(
        BitWriter& bits, 
        std::map<uint32_t, int>& symbol_to_index, 
        const Header& header
    ) {
        bits.add_integer(this->symbols.size(), header.line_count_bit_width);

        for (size_t i = 0; i < this->symbols.size(); i++) {
            int symbol = symbol_to_index[this->symbols[i]];
            int count = this->counts[i];

            bits.add_integer(symbol, header.symbol_bit_width);
            bits.add_integer(count, header.count_bit_width);
        }
    }
};

class DiffRLELine : public BaseRLELine {
public:
    void build(unsigned char * target_line, unsigned char * prev_line, size_t length) {
        uint32_t prev = target_line[0] == prev_line[0] ? SAME_AS_PREV : (uint32_t)target_line[0];
        uint32_t count = 1;

        for (size_t i = 1; i < length; i++) {
            uint32_t symbol = target_line[i] == prev_line[i] ? SAME_AS_PREV : (uint32_t)target_line[i];
            if (symbol == prev) {
                count++;
            } else {
                add(prev, count);
                prev = symbol;
                count = 1;
            }
        }

        add(prev, count);
    }

    void encode(
        BitWriter& bits, 
        std::map<uint32_t, int>& symbol_to_index, 
        const Header& header
    ) {
        int line_len = 0;

        for (size_t i = 0; i < this->symbols.size(); i++) {
            if (this->symbols[i] != SAME_AS_PREV) {
                line_len++;
            }
        }
        
        bits.add_integer(line_len, header.line_count_bit_width);

        int skip_count = 0;
        for (size_t i = 0; i < this->symbols.size(); i++) {
            if (this->symbols[i] == SAME_AS_PREV) {
                skip_count += this->counts[i];
            } else {
                bits.add_integer(skip_count, header.count_bit_width);
                int symbol = symbol_to_index[this->symbols[i]];
                int count = this->counts[i];
                bits.add_integer(symbol, header.symbol_bit_width);
                bits.add_integer(count, header.count_bit_width);
                skip_count = 0;
            }
        }
    }
};


uint8_t estimate_line_count_bit_width(const std::vector<BaseRLELine*>& rle_lines) {
    int max_count = 0;
    for (auto& rle_line : rle_lines) {
        max_count = std::max(max_count, (int)rle_line->symbols.size());
    }

    return get_bit_width(max_count);
}

uint8_t estimate_count_bit_width(const std::vector<BaseRLELine*>& rle_lines) {
    int max_count = 0;
    for (auto& rle_line : rle_lines) {
        for (auto& count : rle_line->counts) {
            max_count = std::max(max_count, (int)count);
        }
    }

    return get_bit_width(max_count);
}

std::vector<uint32_t> get_unique_symbols(const std::vector<BaseRLELine*>& rle_lines) {
    std::set<uint32_t> symbols;

    for (auto& rle_line : rle_lines) {
        for (auto& symbol : rle_line->symbols) {
            if (symbol == SAME_AS_PREV) {
                continue;
            }
            symbols.insert(symbol);
        }
    }

    std::vector<uint32_t> unique_symbols(symbols.begin(), symbols.end());

    return unique_symbols;
}

uint8_t estimate_symbol_bit_width(const std::vector<uint32_t>& unique_symbols) {
    return get_bit_width(unique_symbols.size());
}


/**
 * Encodes a mask using RLE (Run-Length Encoding) algorithm.
 *
 * @param mask The input mask to be encoded.
 * @param shape The shape of the mask, represented as a vector of long integers.
 *              The shape must be 2D.
 * @return The encoded mask as a vector of characters.
 * @throws std::invalid_argument If the mask is not 2D.
 */
std::vector<char> encode_mask(unsigned char * mask, const std::vector<long>& shape) {
    // assert that the mask is 2D
    if (shape.size() != 2) {
        throw std::invalid_argument("Mask must be 2D");
    }

    BitWriter bits;
    std::vector<BaseRLELine *> rle_lines;

    // encode the mask as RLE lines
    // the first line is encoded as a regular RLE line
    PlainRLELine * first_rle_line = new PlainRLELine();
    first_rle_line->build(mask, shape[1]);
    rle_lines.push_back(first_rle_line);

    // the rest of the lines are encoded as RLE lines of differences between the current and the previous line
    for (long i = 1; i < shape[0]; i++) {
        DiffRLELine * rle_line = new DiffRLELine();
        rle_line->build(mask + i * shape[1], mask + (i - 1) * shape[1], shape[1]);
        rle_lines.push_back(rle_line);
    }

    // get the unique symbols in the mask
    auto unique_symbols = get_unique_symbols(rle_lines);

    // estimate the bit widths for counts and symbols
    uint8_t symbol_bit_width = estimate_symbol_bit_width(unique_symbols);
    uint8_t count_bit_width = estimate_count_bit_width(rle_lines);
    uint8_t line_count_bit_width = estimate_line_count_bit_width(rle_lines);

    // create mapping from symbol to index and store it in the encoded mask
    std::map<uint32_t, int> symbol_to_index;
    for (size_t i = 0; i < unique_symbols.size(); i++) {
        symbol_to_index[unique_symbols[i]] = i;
        bits.add_integer(unique_symbols[i], 8);
    }

    // write the header
    Header header = {
        .magic = MAGIC_BYTE,
        .version = VERSION_BYTE,
        .symbol_bit_width = symbol_bit_width,
        .count_bit_width = count_bit_width,
        .line_count_bit_width = line_count_bit_width,
        .unique_symbols_count = static_cast<uint32_t>(unique_symbols.size()),
        .mask_height = static_cast<uint32_t>(shape[0]),
        .mask_width = static_cast<uint32_t>(shape[1])
    };

    std::vector<char> header_bytes(sizeof(Header));
    std::memcpy(header_bytes.data(), &header, sizeof(Header));

    // encode the RLE lines
    for (auto& r : rle_lines) {
        r->encode(bits, symbol_to_index, header);
    }

    std::vector<unsigned char> encoded_data = bits.get_data();

    // pad the encoded data with zeros to align it to the buffer_t size
    while (encoded_data.size() % sizeof(buffer_t) != 0) {
        encoded_data.push_back(static_cast<unsigned char>(0));
    }
    std::vector<char> encoded(encoded_data.begin(), encoded_data.end());
    encoded.insert(encoded.begin(), header_bytes.begin(), header_bytes.end());

    for (auto& r : rle_lines) {
        delete r;
    }

    return encoded;
}
