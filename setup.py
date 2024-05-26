import platform
import cpuinfo
from setuptools import setup, find_packages

from pybind11.setup_helpers import Pybind11Extension, build_ext


os_type = platform.system()
features = cpuinfo.get_cpu_info()

if os_type == "Windows":
    if "avx2" in features["flags"]:
        extra_compile_flags = ["/arch:AVX2"]
    elif "avx" in features["flags"]:
        extra_compile_flags = ["/arch:AVX"]
    elif "sse4_2" in features["flags"]:
        extra_compile_flags = ["/arch:SSE4.2"]
    elif "sse4_1" in features["flags"]:
        extra_compile_flags = ["/arch:SSE4.1"]
    elif "sse3" in features["flags"]:
        extra_compile_flags = ["/arch:SSSE3"]
elif os_type == "Linux":
    extra_compile_flags = ["-march=native"]
elif os_type == "Darwin":
    extra_compile_flags = []


ext_modules = [
    Pybind11Extension(
        name="_pyfastmask",
        sources=["src/wrapper.cpp"],
        include_pybind11=True,
        extra_compile_args=extra_compile_flags
    ),
]

setup(
    cmdclass={"build_ext": build_ext},
    ext_modules=ext_modules,
    packages=find_packages(),
    zip_safe=False,
)
