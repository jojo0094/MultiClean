"""
Example script to compare Python and C++ implementations of utils functions.

This script tests that the C++ module produces identical results to the Python implementation.
"""

import numpy as np
import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from multiclean import utils

def test_create_circle_kernel():
    """Test create_circle_kernel function."""
    print("\n=== Testing create_circle_kernel ===")
    
    try:
        import utils_cpp
        
        for kernel_size in [3, 5, 7, 9]:
            py_kernel = utils.create_circle_kernel(kernel_size)
            cpp_kernel = utils_cpp.create_circle_kernel(kernel_size)
            
            # Compare results
            if np.array_equal(py_kernel, cpp_kernel):
                print(f"✓ kernel_size={kernel_size}: MATCH")
            else:
                print(f"✗ kernel_size={kernel_size}: MISMATCH")
                print(f"  Python:\n{py_kernel}")
                print(f"  C++:\n{cpp_kernel}")
                return False
        
        print("All create_circle_kernel tests passed!")
        return True
    except ImportError:
        print("C++ module not available. Skipping comparison.")
        return None

def test_small_islands_mask_for_class():
    """Test small_islands_mask_for_class function."""
    print("\n=== Testing small_islands_mask_for_class ===")
    
    try:
        import utils_cpp
        
        # Create test image
        image = np.zeros((10, 10), dtype=np.float32)
        image[2, 2] = 1  # Single pixel island
        image[5:7, 5:7] = 1  # 4-pixel island
        
        for min_size in [2, 5]:
            for connectivity in [4, 8]:
                py_mask = utils.small_islands_mask_for_class(image, 1, min_size, connectivity)
                cpp_mask = utils_cpp.small_islands_mask_for_class(image, 1, min_size, connectivity)
                
                # Convert to same dtype for comparison
                py_mask_bool = py_mask.astype(bool)
                cpp_mask_bool = cpp_mask.astype(bool)
                
                if np.array_equal(py_mask_bool, cpp_mask_bool):
                    print(f"✓ min_size={min_size}, connectivity={connectivity}: MATCH")
                else:
                    print(f"✗ min_size={min_size}, connectivity={connectivity}: MISMATCH")
                    print(f"  Python found {py_mask_bool.sum()} small island pixels")
                    print(f"  C++ found {cpp_mask_bool.sum()} small island pixels")
                    return False
        
        print("All small_islands_mask_for_class tests passed!")
        return True
    except ImportError:
        print("C++ module not available. Skipping comparison.")
        return None

def test_smooth_edges():
    """Test smooth_edges function."""
    print("\n=== Testing smooth_edges ===")
    
    try:
        import utils_cpp
        
        # Create test image
        rng = np.random.default_rng(42)
        array = rng.integers(0, 3, size=(20, 20), dtype=np.int32)
        
        target_classes = [0, 1, 2]
        background_classes = []
        
        for smooth_size in [0, 2]:
            py_result = utils.smooth_edges(array, smooth_size, target_classes, background_classes, max_workers=1)
            cpp_result = utils_cpp.smooth_edges(array, smooth_size, target_classes, background_classes)
            
            # Compare results (allowing for small floating point differences)
            # Handle NaN values
            py_nan_mask = np.isnan(py_result)
            cpp_nan_mask = np.isnan(cpp_result)
            
            if not np.array_equal(py_nan_mask, cpp_nan_mask):
                print(f"✗ smooth_size={smooth_size}: NaN masks differ")
                print(f"  Python NaN count: {py_nan_mask.sum()}")
                print(f"  C++ NaN count: {cpp_nan_mask.sum()}")
                return False
            
            # Compare non-NaN values
            non_nan_mask = ~py_nan_mask
            if non_nan_mask.any():
                py_values = py_result[non_nan_mask]
                cpp_values = cpp_result[non_nan_mask]
                
                if np.allclose(py_values, cpp_values, rtol=1e-5):
                    print(f"✓ smooth_size={smooth_size}: MATCH")
                else:
                    print(f"✗ smooth_size={smooth_size}: VALUES MISMATCH")
                    diff = np.abs(py_values - cpp_values)
                    print(f"  Max difference: {diff.max()}")
                    print(f"  Mean difference: {diff.mean()}")
                    return False
            else:
                print(f"✓ smooth_size={smooth_size}: MATCH (all NaN)")
        
        print("All smooth_edges tests passed!")
        return True
    except ImportError:
        print("C++ module not available. Skipping comparison.")
        return None

def test_with_sample_data():
    """Test with sample multiclass data similar to actual use case."""
    print("\n=== Testing with sample multiclass data ===")
    
    try:
        import utils_cpp
        
        # Create realistic sample data
        rng = np.random.default_rng(123)
        array = rng.integers(0, 4, size=(50, 50), dtype=np.int32)
        
        print(f"Sample data shape: {array.shape}")
        print(f"Unique classes: {np.unique(array)}")
        
        # Test create_circle_kernel
        py_kernel = utils.create_circle_kernel(3)
        cpp_kernel = utils_cpp.create_circle_kernel(3)
        print(f"Kernel comparison: {'MATCH' if np.array_equal(py_kernel, cpp_kernel) else 'MISMATCH'}")
        
        # Test small_islands_mask_for_class
        py_islands = utils.small_islands_mask_for_class(array.astype(np.float32), 1, 10, 4)
        cpp_islands = utils_cpp.small_islands_mask_for_class(array.astype(np.float32), 1, 10, 4)
        print(f"Small islands comparison: {'MATCH' if np.array_equal(py_islands.astype(bool), cpp_islands.astype(bool)) else 'MISMATCH'}")
        
        print("\nSample data tests completed!")
        return True
    except ImportError:
        print("C++ module not available. Skipping comparison.")
        return None
    except Exception as e:
        print(f"Error during testing: {e}")
        import traceback
        traceback.print_exc()
        return False

def main():
    """Run all comparison tests."""
    print("=" * 60)
    print("Python vs C++ Implementation Comparison")
    print("=" * 60)
    
    print("\nPython implementation location:", utils.__file__)
    
    try:
        import utils_cpp
        print("C++ module location:", utils_cpp.__file__)
    except ImportError:
        print("\n⚠ C++ module not built yet. Build it with:")
        print("  cd cpp_module")
        print("  conan install . --output-folder=build --build=missing")
        print("  cd build")
        print("  cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake")
        print("  cmake --build .")
        return
    
    # Run tests
    results = []
    results.append(("create_circle_kernel", test_create_circle_kernel()))
    results.append(("small_islands_mask_for_class", test_small_islands_mask_for_class()))
    results.append(("smooth_edges", test_smooth_edges()))
    results.append(("sample_data", test_with_sample_data()))
    
    # Summary
    print("\n" + "=" * 60)
    print("SUMMARY")
    print("=" * 60)
    
    passed = sum(1 for _, result in results if result is True)
    skipped = sum(1 for _, result in results if result is None)
    failed = sum(1 for _, result in results if result is False)
    
    for test_name, result in results:
        status = "✓ PASS" if result is True else ("⊘ SKIP" if result is None else "✗ FAIL")
        print(f"{test_name:40s} {status}")
    
    print(f"\nTotal: {passed} passed, {failed} failed, {skipped} skipped")
    
    if failed > 0:
        sys.exit(1)

if __name__ == "__main__":
    main()
