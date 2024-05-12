from setuptools import setup, find_packages

from pybind11.setup_helpers import Pybind11Extension, build_ext

__version__ = "0.1.0"

extra_compile_flags = ["-march=native"]

ext_modules = [
    Pybind11Extension(
        name="_pyfastmask",
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
    packages=find_packages(),
    zip_safe=False,
    requires=["numpy"],
)
