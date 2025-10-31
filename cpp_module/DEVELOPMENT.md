# C++ Module Development Guide

This document provides detailed information about the C++ implementation of MultiClean utils.

## Architecture

The C++ module (`utils_cpp`) is a Python extension module that provides high-performance implementations of the core utility functions from `multiclean/utils.py`.

### Components

1. **Core C++ Implementation** (`src/utils_cpp.cpp`, `include/utils_cpp.hpp`)
   - Implements the algorithms using OpenCV for image processing
   - Uses standard C++17 features for performance
   - Handles data conversion between NumPy arrays and OpenCV Mat objects

2. **Python Bindings** (in `src/utils_cpp.cpp`)
   - Uses pybind11 to expose C++ functions to Python
   - Automatically converts between NumPy arrays and OpenCV Mat
   - Maintains API compatibility with the Python version

3. **Build System**
   - CMake for cross-platform building
   - Conan for dependency management
   - Supports both Release and Debug builds

## Dependencies

### Required
- **OpenCV 4.x**: Image processing operations
- **pybind11**: Python bindings
- **Python 3.9+**: With NumPy development headers

### Development
- **CMake 3.15+**: Build system
- **Conan**: Package manager
- **C++17 compiler**: GCC 7+, Clang 5+, MSVC 2017+

## Building

### Quick Build
```bash
./build.sh
```

### Manual Build Steps

1. Install Conan dependencies:
```bash
conan install . --output-folder=build --build=missing
```

2. Configure with CMake:
```bash
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
```

3. Build:
```bash
cmake --build . --config Release
```

### Build Options

- **CMAKE_BUILD_TYPE**: Set to `Debug` for debugging symbols, `Release` for optimization
- **CMAKE_TOOLCHAIN_FILE**: Path to Conan toolchain (auto-generated)

## Implementation Details

### Function Mappings

| Python Function | C++ Function | Notes |
|----------------|--------------|-------|
| `small_islands_mask_for_class()` | `utils_cpp::small_islands_mask_for_class()` | Direct port using OpenCV connectedComponentsWithStats |
| `create_circle_kernel()` | `utils_cpp::create_circle_kernel()` | Identical algorithm to Python version |
| `smooth_edges()` | `utils_cpp::smooth_edges()` | Uses OpenCV morphologyEx |
| `find_small_islands()` | `utils_cpp::find_small_islands()` | Parallel processing removed for simplicity |
| `build_invalid_mask()` | `utils_cpp::build_invalid_mask()` | Direct port |
| `fill_invalids()` | `utils_cpp::fill_invalids()` | Simplified nearest-neighbor search |

### Performance Considerations

1. **Memory Management**
   - OpenCV Mat objects use reference counting
   - Avoid unnecessary copies with `clone()` only when needed
   - Python bindings create copies for safety

2. **Parallelization**
   - Current implementation is single-threaded
   - Future work: Add OpenMP or TBB for parallel processing
   - Python version uses ThreadPoolExecutor

3. **Optimization**
   - Compiled with `-O3` in Release mode
   - Uses OpenCV's optimized implementations
   - Consider enabling SIMD instructions for the target platform

## Testing

### Unit Tests
The comparison script in `examples/compare_python_cpp.py` serves as the primary test suite, comparing C++ results against the reference Python implementation.

Run tests:
```bash
python examples/compare_python_cpp.py
```

### Expected Test Results
All tests should show "MATCH" indicating identical results between Python and C++ implementations.

## Known Limitations

1. **fill_invalids()**: The C++ implementation uses a simplified nearest-neighbor search with a limited search radius (50 pixels). For very large gaps, results may differ slightly from the Python version which uses scipy's distance_transform_edt with full precision.

2. **Parallel Processing**: The C++ version does not currently implement parallel processing that the Python version has with ThreadPoolExecutor.

3. **Float Precision**: Minor floating-point differences may occur due to different implementations, but should be within acceptable tolerance (< 1e-5).

## Future Enhancements

1. **Performance**
   - Add OpenMP support for parallel class processing
   - Optimize fill_invalids with better spatial indexing
   - Consider GPU acceleration with CUDA/OpenCL

2. **Features**
   - Support for 3D arrays
   - Additional morphological operations
   - Custom kernel shapes

3. **Packaging**
   - Pre-built wheels for common platforms
   - Integration with PyPI distribution

## Troubleshooting

### Build Errors

**Issue**: `opencv/opencv.hpp not found`
- **Solution**: Ensure Conan installed dependencies correctly. Re-run `conan install`.

**Issue**: `Python.h not found`
- **Solution**: Install Python development headers (`python3-dev` on Ubuntu/Debian).

**Issue**: CMake can't find pybind11
- **Solution**: Ensure Conan's CMake toolchain is being used: `-DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake`

### Runtime Errors

**Issue**: `ImportError: No module named 'utils_cpp'`
- **Solution**: The module should be in the `multiclean` directory. Check build output.

**Issue**: Segmentation fault
- **Solution**: Build in Debug mode and use a debugger. Check array dimensions match.

## Contributing

When adding new functions:

1. Add the C++ declaration to `include/utils_cpp.hpp`
2. Implement in `src/utils_cpp.cpp`
3. Add Python bindings in the `PYBIND11_MODULE` section
4. Update comparison tests in `examples/compare_python_cpp.py`
5. Update documentation

## References

- [pybind11 Documentation](https://pybind11.readthedocs.io/)
- [OpenCV C++ Documentation](https://docs.opencv.org/4.x/)
- [Conan Documentation](https://docs.conan.io/)
- [CMake Documentation](https://cmake.org/documentation/)
