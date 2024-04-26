#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <cstring>


const unsigned char VERSION_BYTE = 0x01;
const unsigned int MAGIC_BYTE = (
      ((unsigned int)'p')
    | ((unsigned int)'f') << 8
    | ((unsigned int)'m') << 16
    | ((unsigned int)'f') << 24
);


template <typename T>
inline T get_integer(char bits, unsigned long long*& data, unsigned char& current_bit_left) {
    static const std::array<unsigned long long, 64> bitmasks = []() {
        std::array<unsigned long long, 64> bitmasks;
        for (char i = 0; i < 64; i++) {
            bitmasks[i] = (1ULL << i) - 1ULL;
        }
        return bitmasks;
    }();

    // assume that bits is less than 64
    if (current_bit_left < bits) {
        T value = data[0] & bitmasks[current_bit_left];
        bits -= current_bit_left;
        value |= (data[1] & bitmasks[bits]) << current_bit_left;
        current_bit_left = 64 - bits;
        data[1] >>= bits;
        data++;
        return value;

    } else {
        T value = data[0] & bitmasks[bits];
        current_bit_left -= bits;
        data[0] >>= bits;
        return value;
    }
}



struct Header {
    unsigned int magic;
    unsigned char version;
    unsigned char symbol_bit_width;
    unsigned char count_bit_width;
    int unique_symbols_count;
    int intervals;
    int mask_height;
    int mask_width;
};


Header read_header(unsigned long long*& data, unsigned char& current_bit_left) {
    Header header;
    header.magic = get_integer<unsigned int>(32, data, current_bit_left);
    header.version = get_integer<unsigned char>(8, data, current_bit_left);
    header.symbol_bit_width = get_integer<unsigned char>(8, data, current_bit_left);
    header.count_bit_width = get_integer<unsigned char>(8, data, current_bit_left);
    header.unique_symbols_count = get_integer<int>(32, data, current_bit_left);
    header.intervals = get_integer<int>(32, data, current_bit_left);
    header.mask_height = get_integer<int>(32, data, current_bit_left);
    header.mask_width = get_integer<int>(32, data, current_bit_left);

    return header;
}


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

    Header header = {
        MAGIC_BYTE,
        VERSION_BYTE,
        symbol_bit_width,
        count_bit_width,
        unique_symbols.size(),
        symbols.size(),
        shape[0],
        shape[1]
    };

    // encode the mask
    BitWriter bits;

    bits.add_integer(header.magic, 32);
    bits.add_integer(header.version, 8);
    bits.add_integer(header.symbol_bit_width, 8);
    bits.add_integer(header.count_bit_width, 8);
    bits.add_integer(header.unique_symbols_count, 32);
    bits.add_integer(header.intervals, 32);
    bits.add_integer(header.mask_height, 32);
    bits.add_integer(header.mask_width, 32);

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


void decode_mask(unsigned long long*& data, Header& header, unsigned char& current_bit_left, unsigned char* mask) {
   
    unsigned char unique_symbols[header.unique_symbols_count];
    for (int i = 0; i < header.unique_symbols_count; ++i) {
        unique_symbols[i] = get_integer<unsigned char>(8, data, current_bit_left);
    }
    
    for (int i = 0; i < header.intervals; ++i) {
        unsigned char symbol_id = get_integer<unsigned char>(header.symbol_bit_width, data, current_bit_left);
        int count = get_integer<int>(header.count_bit_width, data, current_bit_left);
                
        if (symbol_id != 0) {
            memset(mask, unique_symbols[symbol_id], count);
        }
        mask += count;
    }

}
