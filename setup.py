from setuptools import setup

from pybind11.setup_helpers import Pybind11Extension, build_ext

__version__ = "0.1.0"

extra_compile_flags = ["-march=native"]

ext_modules = [
    Pybind11Extension(
        name="pyfastmask",
        sources=["src/wrapper.cpp"],
        include_pybind11=True,
        extra_compile_args=extra_compile_flags
    ),
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
