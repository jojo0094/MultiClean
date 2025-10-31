# Examples

This directory contains example scripts demonstrating various features of MultiClean.

## Available Examples

### compare_python_cpp.py

Compares the Python and C++ implementations of the utility functions to ensure they produce identical results.

**Usage:**
```bash
python examples/compare_python_cpp.py
```

**Prerequisites:**
- The C++ module must be built first (see `cpp_module/README.md`)
- If the C++ module is not available, the script will display build instructions

**What it tests:**
- `create_circle_kernel()` - Tests circular kernel creation with various sizes
- `small_islands_mask_for_class()` - Tests small island detection with different parameters
- `smooth_edges()` - Tests edge smoothing operations
- Sample multiclass data - Tests with realistic data similar to actual use cases

The script will output:
- ✓ for matching results between Python and C++
- ✗ for mismatches (with details about the differences)
- ⊘ for skipped tests (when C++ module is not available)

### using_cpp_module.py

Demonstrates how to use the C++ module directly with practical examples.

**Usage:**
```bash
python examples/using_cpp_module.py
```

**Prerequisites:**
- The C++ module must be built first (see `cpp_module/README.md`)

**Examples included:**
- Creating circular kernels with different sizes
- Detecting small islands in classification arrays
- Smoothing edges with morphological operations
- Complete cleaning workflow using all C++ functions

## Running Examples

Make sure you have installed MultiClean:
```bash
pip install -e .
```

Or if using uv:
```bash
uv sync
```

Then run any example:
```bash
python examples/compare_python_cpp.py
```
