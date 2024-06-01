#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "pybind11/numpy.h"
#include <fstream>
#include <vector>
#include "fastmask.h"
#include "encode.h"
#include "decode.h"

namespace py = pybind11;
using namespace pybind11::literals;


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

    if (size < sizeof(Header)) {
        throw std::invalid_argument("File is not a valid fastmask file. Header is missing.");
    }

    if ((size - sizeof(Header)) % sizeof(buffer_t) != 0) {
        throw std::invalid_argument("File is not a valid fastmask file. Data is not aligned.");
    }

    std::vector<char> buffer(size);

    file.read(buffer.data(), size);
    file.close();

    Header header = read_header(buffer.data());

    if (header.magic != MAGIC_BYTE) {
        throw std::invalid_argument("File is not a valid fastmask file.");
    }

    if (header.version != VERSION_BYTE) {
        throw std::invalid_argument("This file was created with a different version of fastmask.");
    }

    std::vector<unsigned char> mask(header.mask_height * header.mask_width);

    decode_mask(buffer.data() + sizeof(Header), header, mask.data());

    return py::array_t<unsigned char>({header.mask_height, header.mask_width}, mask.data());
}


py::array_t<unsigned char> read_mask_from_bytes(const py::buffer& data_bytes) {
    py::buffer_info info = data_bytes.request();
    
    const char* buffer = static_cast<const char*>(info.ptr);

    if (info.size < sizeof(Header)) {
        throw std::invalid_argument("Data is not a valid fastmask file. Header is missing.");
    }

    if ((info.size - sizeof(Header)) % sizeof(buffer_t) != 0) {
        throw std::invalid_argument("Data is not aligned.");
    }

    Header header = read_header(buffer);

    if (header.magic != MAGIC_BYTE) {
        throw std::invalid_argument("File is not a valid fastmask file.");
    }

    if (header.version != VERSION_BYTE) {
        throw std::invalid_argument("This file was created with a different version of fastmask.");
    }

    std::vector<unsigned char> mask(header.mask_height * header.mask_width);

    decode_mask(buffer + sizeof(Header), header, mask.data());

    return py::array_t<unsigned char>({header.mask_height, header.mask_width}, mask.data());

}

py::dict read_header_from_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();

    if (size < sizeof(Header)) {
        throw std::invalid_argument("File is not a valid fastmask file. Header is missing.");
    }
   
    file.seekg(0, std::ios::beg);

    char buffer[sizeof(Header)];

    file.read(buffer, sizeof(Header));
    file.close();

    Header header = read_header(buffer);

    if (header.magic != MAGIC_BYTE) {
        throw std::invalid_argument("File is not a valid fastmask file.");
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
    m.def("encode", &write_mask_to_bytes, "Encodes mask into buffer", py::arg("mask"));

    m.def("read", &read_mask_from_file, "Read mask from file", py::arg("filename"), py::return_value_policy::move);
    m.def("decode", &read_mask_from_bytes, "Decodes mask from buffer", py::arg("buffer"), py::return_value_policy::move);

    m.def("info", &read_header_from_file, "Read mask header from file", py::arg("filename"));


    py::class_<Header>(m, "Header")
        .def_readonly("symbol_bit_width", &Header::symbol_bit_width)
        .def_readonly("count_bit_width", &Header::count_bit_width)
        .def_readonly("unique_symbols_count", &Header::unique_symbols_count)
        .def_readonly("line_count_bit_width", &Header::line_count_bit_width)
        .def_readonly("mask_height", &Header::mask_height)
        .def_readonly("mask_width", &Header::mask_width);
}

