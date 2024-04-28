from setuptools import setup

from pybind11.setup_helpers import Pybind11Extension, build_ext

__version__ = "2024.04.26"

ext_modules = [
    Pybind11Extension("pyfastmask", ["src/wrapper.cpp"], include_pybind11=True, cxx_std=11),
]

setup(
    name="pyfastmask",
    version=__version__,
    author="Andrei Luzan",
    url="https://github.com/JIy3AHKO/pyfastmask",
    description="Fast low color mask read/write",
    cmdclass={"build_ext": build_ext},
    ext_modules=ext_modules,
    zip_safe=False,
)
