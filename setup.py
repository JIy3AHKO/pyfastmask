from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup

__version__ = "0.0.1"

ext_modules = [
    Pybind11Extension("pyfastmask", ["src/wrapper.cpp"], include_pybind11=True),
]

setup(
    name="pyfastmask",
    version=__version__,
    author="Andrei Luzan",
    cmdclass={"build_ext": build_ext},
    ext_modules=ext_modules,
)