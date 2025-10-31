#!/bin/bash

# Build script for C++ module
# This script builds the utils_cpp Python extension module

set -e

echo "Building C++ module for MultiClean utils..."

# Check if conan is installed
if ! command -v conan &> /dev/null; then
    echo "Error: Conan is not installed. Install it with:"
    echo "  pip install conan"
    exit 1
fi

# Create build directory
mkdir -p build
cd build

# Install dependencies with Conan
echo "Installing dependencies with Conan..."
conan install .. --output-folder=. --build=missing

# Configure with CMake
echo "Configuring with CMake..."
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release

# Build
echo "Building..."
cmake --build . --config Release

echo ""
echo "Build complete! The module has been built in the multiclean directory."
echo "Run 'python examples/compare_python_cpp.py' to test the module."
