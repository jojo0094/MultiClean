# C++ Module for MultiClean Utils

This directory contains a C++ implementation of the `utils.py` module with Python bindings using pybind11.

## Prerequisites

- CMake >= 3.15
- Conan package manager
- C++ compiler with C++17 support
- Python 3.9+

## Installing Conan

If you don't have Conan installed:

```bash
pip install conan
```

## Building the C++ Module

1. Install dependencies with Conan:
```bash
cd cpp_module
conan install . --output-folder=build --build=missing
```

2. Configure and build with CMake:
```bash
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

The compiled module `utils_cpp` will be placed in the `multiclean` directory.

## Testing the Module

Run the comparison script to verify that the C++ implementation produces identical results to the Python implementation:

```bash
cd ..  # Go back to the repository root
python examples/compare_python_cpp.py
```

## Functions Implemented

The following functions from `utils.py` have been implemented in C++:

- `small_islands_mask_for_class()` - Identify small connected components for a single class
- `create_circle_kernel()` - Create a circular morphological kernel
- `smooth_edges()` - Apply morphological opening to smooth edges
- `find_small_islands()` - Detect small connected components for each target class
- `build_invalid_mask()` - Create combined mask of pixels requiring gap filling
- `fill_invalids()` - Fill invalid pixels using nearest-neighbour interpolation

## Performance

The C++ implementation is expected to provide significant performance improvements over the pure Python implementation, especially for large arrays and when processing multiple classes.

## Notes

- The C++ implementation uses OpenCV for image processing operations
- Python bindings are created using pybind11
- The module maintains API compatibility with the original Python implementation
- NumPy arrays are automatically converted to/from OpenCV Mat objects
