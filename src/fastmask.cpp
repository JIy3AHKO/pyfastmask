#include <iostream>
#include <fstream>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "pybind11/numpy.h"


namespace py = pybind11;


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


class BitReader {
private:
    int current_bit_left = 8;
    int vector_position = 0;

public:
    BitReader(std::vector<char>& data) : data(data) {
        vector_position = 0;
        current_bit_left = 8;
    }

    std::vector<char> data;

    template <typename T>
    T get_integer(int bits) {
        T value = 0;
        int value_bits = 0;

        while (value_bits < bits) {
            if (current_bit_left == 0) {
                vector_position++;
                current_bit_left = 8;
            }

            int bits_to_read = std::min(current_bit_left, bits - value_bits);
            value |= (data[vector_position] & ((1 << bits_to_read) - 1)) << value_bits;
            value_bits += bits_to_read;
            data[vector_position] >>= bits_to_read;
            current_bit_left -= bits_to_read;
        }

        return value;

    }

};

    



std::vector<char> encode_mask(unsigned char * mask, size_t size) {
    std::vector<unsigned char> symbols;
    std::vector<int32_t> counts;

    unsigned char prev = mask[0];
    int count = 1;

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
    std::vector<unsigned char> unique_symbols;
    for (size_t i = 0; i < symbols.size(); i++) {
        if (std::find(unique_symbols.begin(), unique_symbols.end(), symbols[i]) == unique_symbols.end()) {
            unique_symbols.push_back(symbols[i]);
        }
    }

    int symbol_bit_width = 1;
    while ((1 << symbol_bit_width) < unique_symbols.size()) {
        symbol_bit_width++;
    }

    // determine the best count bit width
    int count_bit_width = 1;
    int32_t max_count = *std::max_element(counts.begin(), counts.end());

    while ((1 << count_bit_width) < max_count) {
        count_bit_width++;
    }

    // encode the mask
    BitWriter bits;

    # ifdef DEBUG
    std::cerr << "symbol_bit_width: " << symbol_bit_width << std::endl;
    std::cerr << "count_bit_width: " << count_bit_width << std::endl;

    # endif

    bits.add_integer(symbol_bit_width, 8);
    bits.add_integer(count_bit_width, 8);
    bits.add_integer(unique_symbols.size(), 32);
    bits.add_integer(symbols.size(), 32);
    bits.add_integer(size, 32);

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

    # ifdef DEBUG
    
    std::cerr << "encoded size: " << encoded.size() << std::endl;

    # endif

    return encoded;
}


std::vector<unsigned char> decode_mask(std::vector<char>& encoded) {
    BitReader bits(encoded);

    # ifdef DEBUG
    std::cerr << "Reading mask..." << std::endl;
    # endif

    int symbol_bit_width = bits.get_integer<int>(8);
    int count_bit_width = bits.get_integer<int>(8);
    int unique_symbols_count = bits.get_integer<int>(32);
    int intervals = bits.get_integer<int>(32);
    int mask_size = bits.get_integer<int>(32);
    
    # ifdef DEBUG

    std::cerr << "symbol_bit_width: " << symbol_bit_width << std::endl;
    std::cerr << "count_bit_width: " << count_bit_width << std::endl;
    std::cerr << "unique_symbols_count: " << unique_symbols_count << std::endl;
    std::cerr << "intervals: " << intervals << std::endl;

    # endif

    std::vector<unsigned char> unique_symbols;
    for (int i = 0; i < unique_symbols_count; i++) {
        unique_symbols.push_back(bits.get_integer<unsigned char>(8));
    }

    std::map<int, unsigned char> index_to_symbol;
    for (size_t i = 0; i < unique_symbols.size(); i++) {
        index_to_symbol[i] = unique_symbols[i];
    }

    std::vector<unsigned char> mask(mask_size);

    int mask_index = 0;
    for (int i = 0; i < intervals; i++) {
        int symbol = bits.get_integer<int>(symbol_bit_width);
        int count = bits.get_integer<int>(count_bit_width);

        for (int j = 0; j < count; j++) {
            mask[mask_index] = index_to_symbol[symbol];
            mask_index++;
        }
    }

    return mask;
}






void writeMask(const std::string& filename, py::buffer mask) {
    py::buffer_info info = mask.request();
    unsigned char* ptr = static_cast<unsigned char*>(info.ptr);
    size_t size = info.size;

    std::vector<char> encoded = encode_mask(ptr, size);

    std::ofstream file(filename, std::ios::binary);
    file.write(encoded.data(), encoded.size());
    file.close();
}


py::array_t<unsigned char> readMask(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    std::vector<char> encoded((std::istreambuf_iterator<char >(file)), std::istreambuf_iterator<char>());
    file.close();

    std::vector<unsigned char> mask = decode_mask(encoded);

    return py::array_t<unsigned char>(mask.size(), mask.data());
}


PYBIND11_MODULE(pyfastmask, m) {
    m.doc() = "Fast mask module";
    m.def("writeMask", &writeMask, "Write mask to file");
    m.def("readMask", &readMask, "Read mask from file", py::return_value_policy::move);
}

