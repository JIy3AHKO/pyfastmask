#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "pybind11/numpy.h"
#include "fastmask.h"


namespace py = pybind11;


void writeMask(const std::string& filename, py::buffer mask) {
    py::buffer_info info = mask.request();
    unsigned char* ptr = static_cast<unsigned char*>(info.ptr);

    std::vector<char> encoded = encode_mask(ptr, info.shape);

    std::ofstream file(filename, std::ios::binary);
    file.write(encoded.data(), encoded.size());
    file.close();
}


py::array_t<unsigned char> readMask(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    std::streamsize size = file.tellg();
    auto padded_size = size + (8 - (size % 8));
    file.seekg(0, std::ios::beg);

    char buffer[padded_size];

    file.read(buffer, size);
    file.close();
    
    unsigned long long* data = reinterpret_cast<unsigned long long*>(buffer);
    unsigned char current_bit_left = 64;

    Header header = read_header(data, current_bit_left);

    if (header.magic != MAGIC_BYTE) {
        throw std::invalid_argument("Invalid magic byte");
    }

    if (header.version != VERSION_BYTE) {
        throw std::invalid_argument("Invalid version byte");
    }

    unsigned char unique_symbols[header.unique_symbols_count];

    for (int i = 0; i < header.unique_symbols_count; ++i) {
        get_integer<unsigned char>(8, data, current_bit_left, unique_symbols[i]);
    }
    
    unsigned char mask[header.mask_height * header.mask_width];
    memset(mask, unique_symbols[0], header.mask_height * header.mask_width);


    decode_mask(data, header, current_bit_left, mask, unique_symbols);

    return py::array_t<unsigned char>({header.mask_height, header.mask_width}, mask);
}

Header readHeader(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    std::streamsize size = file.tellg();
   
    file.seekg(0, std::ios::beg);

    char buffer[sizeof(Header)];

    file.read(buffer, sizeof(Header));

    file.close();

    unsigned long long* data = reinterpret_cast<unsigned long long*>(buffer);

    unsigned char current_bit_left = 64;

    return read_header(data, current_bit_left);
}
    


PYBIND11_MODULE(pyfastmask, m) {
    m.doc() = "Fast mask module";

    m.def("write", &writeMask, "Write mask to file", py::arg("filename"), py::arg("mask"));
    m.def("read", &readMask, "Read mask from file", py::arg("filename"), py::return_value_policy::move);
    m.def("info", &readHeader, "Read mask header from file", py::arg("filename"));

    py::class_<Header>(m, "Header")
        .def_readonly("symbol_bit_width", &Header::symbol_bit_width)
        .def_readonly("count_bit_width", &Header::count_bit_width)
        .def_readonly("unique_symbols_count", &Header::unique_symbols_count)
        .def_readonly("intervals", &Header::intervals)
        .def_readonly("mask_height", &Header::mask_height)
        .def_readonly("mask_width", &Header::mask_width);

    
}

