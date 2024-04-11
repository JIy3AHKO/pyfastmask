#include <iostream>
#include <fstream>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "pybind11/numpy.h"


namespace py = pybind11;



void writeMask(const std::string& filename, py::buffer mask) {
    py::buffer_info info = mask.request();
    char* ptr = static_cast<char*>(info.ptr);
    std::ofstream file(filename, std::ios::binary);
    file.write(ptr, info.size);
}


py::array_t<char> readMask(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> buffer(size);
    file.read(buffer.data(), size);
    py::array_t<char> mask(size);
    py::buffer_info info = mask.request();
    std::memcpy(info.ptr, buffer.data(), size);

    return mask;
}


PYBIND11_MODULE(pyfastmask, m) {
    m.doc() = "Fast mask module";
    m.def("writeMask", &writeMask, "Write mask to file");
    m.def("readMask", &readMask, "Read mask from file", py::return_value_policy::move);
}

