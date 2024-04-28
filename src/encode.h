#pragma once
#include <vector>
#include <map>
#include <algorithm>
#include "fastmask.h"

class BitWriter {
private:
    char running_byte = 0; // byte that is being built
    int running_bits = 0; // number of bits in the running byte

public:
    BitWriter() {}
    std::vector<unsigned char> data;

    void add_integer(uint64_t value, int value_bits) {
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



std::vector<unsigned char> generate_unique_symbols_map(std::vector<unsigned char>& symbols) {
    // generate unique symbols map putting most frequent symbols first
    std::map<unsigned char, int> symbol_to_count;
    for (unsigned char symbol : symbols) {
        symbol_to_count[symbol]++;
    }

    std::vector<std::pair<unsigned char, int>> symbol_count_pairs;
    for (auto& pair : symbol_to_count) {
        symbol_count_pairs.push_back(pair);
    }

    std::sort(symbol_count_pairs.begin(), symbol_count_pairs.end(), [](const std::pair<unsigned char, int>& a, const std::pair<unsigned char, int>& b) {
        return a.second > b.second;
    });

    std::vector<unsigned char> unique_symbols;
    for (auto& pair : symbol_count_pairs) {
        unique_symbols.push_back(pair.first);
    }

    return unique_symbols;
}
    



std::vector<char> encode_mask(unsigned char * mask, std::vector<long>& shape) {
    std::vector<unsigned char> symbols;
    std::vector<uint32_t> counts;

    // assert that the mask is 2D
    if (shape.size() != 2) {
        throw std::invalid_argument("Mask must be 2D");
    }

    size_t size = shape[0] * shape[1];

    unsigned char prev = mask[0];
    uint32_t count = 1;

    for (size_t i = 1; i < size; i++) {
        if (mask[i] == prev) {
            count++;
        } else {
            symbols.push_back(prev);
            counts.push_back(count);
            prev = mask[i];
            count = 1;
        }
    }
    
    symbols.push_back(prev);
    counts.push_back(count);

    // determine unique symbols and choose the best symbol bit width
    std::vector<unsigned char> unique_symbols = generate_unique_symbols_map(symbols);

    unsigned char symbol_bit_width = 1;
    while ((1ULL << symbol_bit_width) < unique_symbols.size()) {
        symbol_bit_width++;
    }

    // determine the best count bit width
    unsigned char count_bit_width = 1;
    uint32_t max_count = *std::max_element(counts.begin(), counts.end());

    while ((1ULL << count_bit_width) <= max_count) {
        count_bit_width++;
    }

    Header header = {
        MAGIC_BYTE,
        VERSION_BYTE,
        symbol_bit_width,
        count_bit_width,
        (unsigned int)unique_symbols.size(),
        (unsigned int)symbols.size(),
        shape[0],
        shape[1]
    };

    // encode the mask
    BitWriter bits;

    // create mapping from symbol to index and store it in the encoded mask
    std::map<unsigned char, int> symbol_to_index;
    for (size_t i = 0; i < unique_symbols.size(); i++) {
        symbol_to_index[unique_symbols[i]] = i;
        bits.add_integer(unique_symbols[i], 8);
    }

    // encode the mask
    for (size_t i = 0; i < symbols.size(); i++) {
        int symbol = symbol_to_index[symbols[i]];
        int count = counts[i];

        bits.add_integer(symbol, symbol_bit_width);
        bits.add_integer(count, count_bit_width);
    }

    std::vector<char> header_bytes(sizeof(Header));
    memcpy(header_bytes.data(), &header, sizeof(Header));

    std::vector<unsigned char> encoded_data = bits.get_data();
    std::vector<char> encoded(encoded_data.begin(), encoded_data.end());
    encoded.insert(encoded.begin(), header_bytes.begin(), header_bytes.end());

    return encoded;
}
