#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "pybind11/numpy.h"
#include <fstream>
#include <vector>
#include <stdexcept>
#include <string>
#include "fastmask.h"
#include "encode.h"
#include "decode.h"

namespace py = pybind11;
using namespace pybind11::literals;

void validate_header(const Header& header) {
    if (header.magic != MAGIC_BYTE) {
        throw std::invalid_argument("File is not a valid fastmask file.");
    }

    if (header.version != VERSION_BYTE) {
        throw std::invalid_argument("This file was created with a different version of fastmask.");
    }
}

void validate_buffer_size(size_t size) {
    if (size < sizeof(Header)) {
        throw std::invalid_argument("Data is not a valid fastmask file. Header is missing.");
    }

    if ((size - sizeof(Header)) % sizeof(buffer_t) != 0) {
        throw std::invalid_argument("Data is not aligned.");
    }
}

std::invalid_argument add_filename_to_error(const std::string& filename, std::invalid_argument& e) {
    return std::invalid_argument("Error while reading file " + filename + ": " + e.what());
}

void write_mask_to_file(const std::string& filename, py::buffer mask) {
    py::buffer_info info = mask.request();
    unsigned char* ptr = static_cast<unsigned char*>(info.ptr);

    std::vector<long> shape = std::vector<long>(info.shape.begin(), info.shape.end());
    std::vector<char> encoded = encode_mask(ptr, shape);

    std::ofstream file(filename, std::ios::binary);
    file.write(encoded.data(), encoded.size());
    file.close();
}

py::bytes write_mask_to_bytes(py::buffer mask) {
    py::buffer_info info = mask.request();
    unsigned char* ptr = static_cast<unsigned char*>(info.ptr);

    std::vector<long> shape = std::vector<long>(info.shape.begin(), info.shape.end());
    std::vector<char> encoded = encode_mask(ptr, shape);

    return py::bytes(encoded.data(), encoded.size());
}


py::array_t<unsigned char> read_mask_from_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    try {
        validate_buffer_size(size);
    } catch (std::invalid_argument& e) {
        throw add_filename_to_error(filename, e);
    }

    std::vector<char> buffer(size);

    file.read(buffer.data(), size);
    file.close();

    Header header = read_header(buffer.data());
    try {
        validate_header(header);
    } catch (std::invalid_argument& e) {
        throw add_filename_to_error(filename, e);
    }

    std::vector<unsigned char> mask(header.mask_height * header.mask_width);

    decode_mask(buffer.data() + sizeof(Header), header, mask.data());

    return py::array_t<unsigned char>({header.mask_height, header.mask_width}, mask.data());
}


py::array_t<unsigned char> read_mask_from_buffer(const py::buffer& data_bytes) {
    py::buffer_info info = data_bytes.request();
    
    const char* buffer = static_cast<const char*>(info.ptr);

    validate_buffer_size(info.size);

    Header header = read_header(buffer);
    validate_header(header);

    std::vector<unsigned char> mask(header.mask_height * header.mask_width);

    decode_mask(buffer + sizeof(Header), header, mask.data());

    return py::array_t<unsigned char>({header.mask_height, header.mask_width}, mask.data());

}

py::dict read_header_from_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();

    try {
        validate_buffer_size(size);
    } catch (std::invalid_argument& e) {
        throw add_filename_to_error(filename, e);
    }
   
    file.seekg(0, std::ios::beg);

    char buffer[sizeof(Header)];

    file.read(buffer, sizeof(Header));
    file.close();

    Header header = read_header(buffer);
    try {
        validate_header(header);
    } catch (std::invalid_argument& e) {
        throw add_filename_to_error(filename, e);
    }

    return py::dict(
        "symbol_bit_width"_a = header.symbol_bit_width,
        "count_bit_width"_a = header.count_bit_width,
        "unique_symbols_count"_a = header.unique_symbols_count,
        "line_count_bit_width"_a = header.line_count_bit_width,
        "shape"_a = py::make_tuple(header.mask_height, header.mask_width)
    );
}
    


PYBIND11_MODULE(_pyfastmask, m) {
    m.doc() = "Fast mask module";

    m.def("write", &write_mask_to_file, "Write mask to file", py::arg("filename"), py::arg("mask"));
    m.def("encode", &write_mask_to_bytes, "Encodes mask into bytes object", py::arg("mask"));

    m.def("read", &read_mask_from_file, "Read mask from file", py::arg("filename"), py::return_value_policy::move);
    m.def("decode", &read_mask_from_buffer, "Decodes mask from buffer", py::arg("buffer"), py::return_value_policy::move);

    m.def("info", &read_header_from_file, "Read mask header from file", py::arg("filename"));


    py::class_<Header>(m, "Header")
        .def_readonly("symbol_bit_width", &Header::symbol_bit_width)
        .def_readonly("count_bit_width", &Header::count_bit_width)
        .def_readonly("unique_symbols_count", &Header::unique_symbols_count)
        .def_readonly("line_count_bit_width", &Header::line_count_bit_width)
        .def_readonly("mask_height", &Header::mask_height)
        .def_readonly("mask_width", &Header::mask_width);
}

