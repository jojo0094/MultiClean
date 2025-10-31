cmake_minimum_required(VERSION 3.15)
project(utils_cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Python
find_package(Python COMPONENTS Interpreter Development NumPy REQUIRED)

# Include Conan-generated toolchain
if(EXISTS ${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
    include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
endif()

# Find packages
find_package(OpenCV REQUIRED)
find_package(pybind11 REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${Python_INCLUDE_DIRS}
    ${Python_NumPy_INCLUDE_DIRS}
)

# Source files
set(SOURCES
    src/utils_cpp.cpp
)

# Create Python module
pybind11_add_module(utils_cpp ${SOURCES})

# Link libraries
target_link_libraries(utils_cpp PRIVATE
    ${OpenCV_LIBS}
    Python::NumPy
)

# Set output directory
set_target_properties(utils_cpp PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../multiclean
)
