"""
Example demonstrating how to use the C++ module directly.

This script shows how to call the C++ functions directly instead of
going through the Python wrapper layer.
"""

import numpy as np

def main():
    print("=" * 60)
    print("Using the C++ Module")
    print("=" * 60)
    
    try:
        import utils_cpp
        print("\n✓ C++ module loaded successfully!")
        print(f"Module location: {utils_cpp.__file__}\n")
    except ImportError:
        print("\n✗ C++ module not available.")
        print("Build it first with:")
        print("  cd cpp_module && ./build.sh\n")
        return
    
    # Example 1: Create a circular kernel
    print("Example 1: Creating circular kernels")
    print("-" * 60)
    for size in [3, 5, 7]:
        kernel = utils_cpp.create_circle_kernel(size)
        print(f"\nKernel size {size}x{size}:")
        print(kernel)
    
    # Example 2: Detect small islands
    print("\n\nExample 2: Detecting small islands")
    print("-" * 60)
    
    # Create a test image with islands
    image = np.zeros((15, 15), dtype=np.float32)
    image[2:4, 2:4] = 1.0      # 4-pixel island
    image[7, 7] = 1.0           # 1-pixel island
    image[10:13, 10:13] = 1.0   # 9-pixel island
    
    print("\nTest image (class 1 islands):")
    print(image.astype(int))
    
    # Detect islands smaller than 5 pixels
    small_islands = utils_cpp.small_islands_mask_for_class(
        image, class_value=1, min_size=5, connectivity=4
    )
    
    print("\nSmall islands mask (min_size=5):")
    print(small_islands.astype(int))
    print(f"Found {small_islands.sum()} pixels in small islands")
    
    # Example 3: Smooth edges
    print("\n\nExample 3: Smoothing edges")
    print("-" * 60)
    
    # Create a test image with multiple classes
    rng = np.random.default_rng(42)
    array = rng.integers(0, 3, size=(20, 20), dtype=np.int32)
    
    print(f"\nOriginal array shape: {array.shape}")
    print(f"Classes: {np.unique(array)}")
    
    # Smooth edges for classes 0, 1, 2
    smoothed = utils_cpp.smooth_edges(
        array,
        smooth_edge_size=2,
        target_class_values=[0, 1, 2],
        background_class_values=[]
    )
    
    print(f"\nSmoothed array shape: {smoothed.shape}")
    print(f"Number of NaN pixels: {np.isnan(smoothed).sum()}")
    print(f"Classes in smoothed array: {np.unique(smoothed[~np.isnan(smoothed)])}")
    
    # Example 4: Complete workflow
    print("\n\nExample 4: Complete cleaning workflow")
    print("-" * 60)
    
    # Create sample data
    sample_data = np.zeros((30, 30), dtype=np.int32)
    sample_data[:15, :15] = 0
    sample_data[:15, 15:] = 1
    sample_data[15:, :15] = 2
    sample_data[15:, 15:] = 3
    
    # Add some noise (small islands)
    rng = np.random.default_rng(123)
    noise_positions = rng.choice(900, size=20, replace=False)
    for pos in noise_positions:
        y, x = pos // 30, pos % 30
        sample_data[y, x] = (sample_data[y, x] + 1) % 4
    
    print(f"\nSample data with noise: {sample_data.shape}")
    print(f"Classes: {np.unique(sample_data)}")
    
    # Step 1: Smooth edges
    smoothed = utils_cpp.smooth_edges(
        sample_data,
        smooth_edge_size=2,
        target_class_values=[0, 1, 2, 3],
        background_class_values=[]
    )
    print(f"\nAfter smoothing: {np.isnan(smoothed).sum()} NaN pixels")
    
    # Step 2: Find small islands
    small_islands_dict = utils_cpp.find_small_islands(
        smoothed,
        target_class_values=[0, 1, 2, 3],
        min_island_size=10,
        connectivity=4
    )
    
    total_small_island_pixels = sum(mask.sum() for mask in small_islands_dict.values())
    print(f"Small islands found: {total_small_island_pixels} pixels across {len(small_islands_dict)} classes")
    
    # Step 3: Build invalid mask
    invalid_mask = utils_cpp.build_invalid_mask(smoothed, small_islands_dict)
    print(f"Total invalid pixels: {invalid_mask.sum()}")
    
    # Step 4: Fill invalids
    filled = utils_cpp.fill_invalids(smoothed, invalid_mask, [0, 1, 2, 3])
    print(f"\nAfter filling: {np.isnan(filled).sum()} NaN pixels")
    print(f"Final classes: {np.unique(filled[~np.isnan(filled)])}")
    
    print("\n" + "=" * 60)
    print("All examples completed successfully!")
    print("=" * 60)

if __name__ == "__main__":
    main()
