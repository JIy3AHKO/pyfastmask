#pragma once
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <tuple>
#include <string>
#include <stdexcept>
#include "fastmask.h"

const uint32_t SAME_AS_PREV = (uint32_t)-1;


class BitWriter {
private:
    char running_byte = 0; // byte that is being built
    int running_bits = 0; // number of bits in the running byte

public:
    BitWriter() {}
    std::vector<unsigned char> data;

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

    std::vector<unsigned char> get_data() {
        if (running_bits > 0) {
            data.push_back(running_byte);
        }
        return data;
    }

};



std::vector<uint32_t> generate_unique_symbols_map(std::vector<uint32_t>& symbols) {
    std::set<uint32_t> symbol_to_count;
    for (auto symbol : symbols) {
        if (symbol == SAME_AS_PREV) {
            continue;
        }
        symbol_to_count.insert(symbol);
    }

    std::vector<uint32_t> unique_symbols;
    for (auto symbol : symbol_to_count) {
        unique_symbols.push_back(symbol);
    }

    return unique_symbols;
}



struct RLELine {
    std::vector<uint32_t> symbols;
    std::vector<uint32_t> counts;
    bool is_diff = false;

    void add(uint32_t symbol, uint32_t count) {
        // printf("Adding symbol %d with count %d\n", symbol, count);
        symbols.push_back(symbol);
        counts.push_back(count);
    }

    void clear() {
        symbols.clear();
        counts.clear();
    }

    void build(unsigned char * line, size_t length) {
        uint32_t prev = line[0];
        uint32_t count = 1;
        is_diff = false;

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


    void build_diff(unsigned char * target_line, unsigned char * prev_line, size_t length) {
        uint32_t prev = target_line[0] == prev_line[0] ? SAME_AS_PREV : (uint32_t)target_line[0];
        uint32_t count = 1;
        is_diff = true;

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
        int symbol_bit_width, 
        int count_bit_width,
        int line_count_bit_width
    ) {
        if (is_diff) {
            encode_diff(bits, symbol_to_index, symbol_bit_width, count_bit_width, line_count_bit_width);
        } else {
            encode_plain(bits, symbol_to_index, symbol_bit_width, count_bit_width, line_count_bit_width);
        }
    }

    void encode_plain(
        BitWriter& bits, 
        std::map<uint32_t, int>& symbol_to_index, 
        int symbol_bit_width, 
        int count_bit_width,
        int line_count_bit_width
    ) {
        bits.add_integer(this->symbols.size(), line_count_bit_width);

        for (size_t i = 0; i < this->symbols.size(); i++) {
            int symbol = symbol_to_index[this->symbols[i]];
            int count = this->counts[i];

            bits.add_integer(symbol, symbol_bit_width);
            bits.add_integer(count, count_bit_width);
        }
    }

    void encode_diff(
        BitWriter& bits, 
        std::map<uint32_t, int>& symbol_to_index, 
        int symbol_bit_width, 
        int count_bit_width,
        int line_count_bit_width
    ) {
        // diff is encoded as 
        // [skip_count, symbol, count] where skip_count is the number of symbols that are the same as the previous line
        // and symbol and count are the symbol and count of the first symbol that is different from the previous line

        int line_len = 0;

        for (size_t i = 0; i < this->symbols.size(); i++) {
            if (this->symbols[i] != SAME_AS_PREV) {
                line_len++;
            }
        }

        
        bits.add_integer(line_len, line_count_bit_width);

        int skip_count = 0;
        for (size_t i = 0; i < this->symbols.size(); i++) {
            if (this->symbols[i] == SAME_AS_PREV) {
                skip_count += this->counts[i];
            } else {
                bits.add_integer(skip_count, count_bit_width);
                int symbol = symbol_to_index[this->symbols[i]];
                int count = this->counts[i];
                bits.add_integer(symbol, symbol_bit_width);
                bits.add_integer(count, count_bit_width);
                skip_count = 0;
            }
        }
    }


};


uint8_t estimate_line_count_bit_width(const std::vector<RLELine>& rle_lines) {
    int max_count = 0;
    for (auto& rle_line : rle_lines) {
        max_count = std::max(max_count, (int)rle_line.symbols.size());
    }

    uint8_t line_count_bit_width = 1;
    while ((1 << line_count_bit_width) <= max_count) {
        line_count_bit_width++;
    }

    return line_count_bit_width;
}

uint8_t estimate_count_bit_width(const std::vector<RLELine>& rle_lines) {
    int max_count = 0;
    for (auto& rle_line : rle_lines) {
        for (auto& count : rle_line.counts) {
            max_count = std::max(max_count, (int)count);
        }
    }

    uint8_t count_bit_width = 1;
    while ((1 << count_bit_width) <= max_count) {
        count_bit_width++;
    }

    return count_bit_width;
}

std::vector<uint32_t> generate_all_unique_symbols(const std::vector<RLELine>& rle_lines) {
    std::vector<uint32_t> all_symbols;
    for (auto& rle_line : rle_lines) {
        for (auto& symbol : rle_line.symbols) {
            all_symbols.push_back(symbol);
        }
    }

    return generate_unique_symbols_map(all_symbols);
}

uint8_t estimate_symbol_bit_width(const std::vector<uint32_t>& unique_symbols) {
    uint8_t symbol_bit_width = 1;
    while ((1 << symbol_bit_width) < unique_symbols.size()) {
        symbol_bit_width++;
    }

    return symbol_bit_width;
}


std::vector<char> encode_mask(unsigned char * mask, std::vector<long>& shape) {
    // assert that the mask is 2D
    if (shape.size() != 2) {
        throw std::invalid_argument("Mask must be 2D");
    }

    BitWriter bits;

    std::vector<RLELine> rle_lines;

    RLELine first_rle_line;
    first_rle_line.build(mask, shape[1]);
    rle_lines.push_back(first_rle_line);

    for (size_t i = 1; i < shape[0]; i++) {
        RLELine rle_line;
        rle_line.build_diff(mask + i * shape[1], mask + (i - 1) * shape[1], shape[1]);
        rle_lines.push_back(rle_line);
    }

    auto unique_symbols = generate_all_unique_symbols(rle_lines);
    uint8_t symbol_bit_width = estimate_symbol_bit_width(unique_symbols);
    uint8_t count_bit_width = estimate_count_bit_width(rle_lines);
    uint8_t line_count_bit_width = estimate_line_count_bit_width(rle_lines);

    // create mapping from symbol to index and store it in the encoded mask
    std::map<uint32_t, int> symbol_to_index;
    for (size_t i = 0; i < unique_symbols.size(); i++) {
        symbol_to_index[unique_symbols[i]] = i;
        bits.add_integer(unique_symbols[i], 8);
    }

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

    for (auto& r : rle_lines) {
        r.encode(bits, symbol_to_index, symbol_bit_width, count_bit_width, line_count_bit_width);
    }

    std::vector<char> header_bytes(sizeof(Header));
    memcpy(header_bytes.data(), &header, sizeof(Header));

    std::vector<unsigned char> encoded_data = bits.get_data();
    std::vector<char> encoded(encoded_data.begin(), encoded_data.end());
    encoded.insert(encoded.begin(), header_bytes.begin(), header_bytes.end());

    return encoded;
}
