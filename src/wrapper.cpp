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

    std::vector<char> buffer(padded_size);

    file.read(buffer.data(), size);
    file.close();


    std::vector<unsigned long long> data(
            reinterpret_cast<unsigned long long*>(buffer.data()),
            reinterpret_cast<unsigned long long*>(buffer.data()) + padded_size / sizeof(unsigned long long)
    );

    int mask_height, mask_width;
    std::vector<unsigned char> mask = decode_mask(data, mask_height, mask_width);

    return py::array_t<unsigned char>({mask_height, mask_width}, mask.data());
}


PYBIND11_MODULE(pyfastmask, m) {
    m.doc() = "Fast mask module";
    m.def("write", &writeMask, "Write mask to file");
    m.def("read", &readMask, "Read mask from file", py::return_value_policy::move);
}

