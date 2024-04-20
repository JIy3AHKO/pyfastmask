#include <iostream>
#include <fstream>
#include <vector>
#include <map>


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
        // pad the data with zeros to be aligned to 8 bytes
        while (data.size() % 8 != 0) {
            data.push_back(0);
        }
        return data;
    }

};


class BitReader {
private:
    char current_bit_left = 64;
    long vector_position = 0;
    std::array<unsigned long long, 64> bitmasks;

public:
    BitReader(std::vector<unsigned long long>& data) : data(data) {
        vector_position = 0;
        current_bit_left = 64;
        for (char i = 0; i < 64; i++) {
            bitmasks[i] = (1ULL << i) - 1ULL;
        }
    }

    std::vector<unsigned long long>& data;

    template <typename T>
    T get_integer(char bits) {
        // assume that bits is less than 64
        if (current_bit_left < bits) {
            T value = data[vector_position] & bitmasks[current_bit_left];
            bits -= current_bit_left;
            vector_position++;
            value |= (data[vector_position] & bitmasks[bits]) << current_bit_left;
            current_bit_left = 64 - bits;
            data[vector_position] >>= bits;
            return value;

        } else {
            T value = data[vector_position] & bitmasks[bits];
            current_bit_left -= bits;
            data[vector_position] >>= bits;
            return value;
        }
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

    int symbol_bit_width = 1;
    while ((1ULL << symbol_bit_width) < unique_symbols.size()) {
        symbol_bit_width++;
    }

    // determine the best count bit width
    int count_bit_width = 1;
    uint32_t max_count = *std::max_element(counts.begin(), counts.end());

    while ((1ULL << count_bit_width) <= max_count) {
        count_bit_width++;
    }

    // encode the mask
    BitWriter bits;

    bits.add_integer(symbol_bit_width, 8);
    bits.add_integer(count_bit_width, 8);
    bits.add_integer(unique_symbols.size(), 32);
    bits.add_integer(symbols.size(), 32);
    bits.add_integer(shape[0], 32);
    bits.add_integer(shape[1], 32);

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

    std::vector<unsigned char> encoded_data = bits.get_data();
    std::vector<char> encoded(encoded_data.begin(), encoded_data.end());

    return encoded;
}


std::vector<unsigned char> decode_mask(std::vector<unsigned long long>& encoded, int& mask_height, int& mask_width) {
    BitReader bits(encoded);

    int symbol_bit_width = bits.get_integer<int>(8);
    int count_bit_width = bits.get_integer<int>(8);
    int unique_symbols_count = bits.get_integer<int>(32);
    int intervals = bits.get_integer<int>(32);
    mask_height = bits.get_integer<int>(32);
    mask_width = bits.get_integer<int>(32);
    int mask_size = mask_height * mask_width;
    

    std::vector<unsigned char> unique_symbols(unique_symbols_count);
    for (int i = 0; i < unique_symbols_count; ++i) {
        unique_symbols[i] = bits.get_integer<unsigned char>(8);
    }

    std::vector<unsigned char> mask(mask_size, unique_symbols[0]);
    int mask_index = 0;
    for (int i = 0; i < intervals; ++i) {
        int symbol = bits.get_integer<int>(symbol_bit_width);
        int count = bits.get_integer<int>(count_bit_width);
                
        if (symbol != 0) {
            for (int j = 0; j < count; j++) {
                mask[mask_index] = unique_symbols[symbol];
                mask_index++;
            }
        } else {
            mask_index += count;
        }

    }

    return mask;
}
