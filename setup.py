import platform
import cpufeature
from setuptools import setup, find_packages

from pybind11.setup_helpers import Pybind11Extension, build_ext

__version__ = "0.1.0"

os_type = platform.system()
features = cpufeature.CPUFeature

if os_type == "Windows":
    if features["AVX2"]:
        extra_compile_flags = ["/arch:AVX2"]
    elif features["AVX"]:
        extra_compile_flags = ["/arch:AVX"]
    elif features["SSE4_2"]:
        extra_compile_flags = ["/arch:SSE4.2"]
    elif features["SSE4_1"]:
        extra_compile_flags = ["/arch:SSE4.1"]
    elif features["SSSE3"]:
        extra_compile_flags = ["/arch:SSSE3"]
elif os_type == "Linux":
    extra_compile_flags = ["-march=native"]
elif os_type == "Darwin":
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
