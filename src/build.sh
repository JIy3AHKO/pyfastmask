rm *.so
c++ -O3 -Wall -shared -std=c++11 -fPIC $(python3 -m pybind11 --includes) fastmask.cpp -o pyfastmask$(python3-config --extension-suffix)
python3 -m unittest discover