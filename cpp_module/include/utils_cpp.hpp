#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <map>
#include <tuple>

namespace utils_cpp {

/**
 * Identify small connected components for a single class using area analysis.
 * 
 * @param image Classification array
 * @param class_value Specific class value to analyse
 * @param min_size Minimum area threshold for connected components
 * @param connectivity Pixel connectivity (4 or 8)
 * @return Boolean mask indicating small connected components
 */
cv::Mat small_islands_mask_for_class(
    const cv::Mat& image,
    int class_value,
    int min_size,
    int connectivity
);

/**
 * Create a circular morphological kernel.
 * 
 * @param kernel_size Size of the square kernel
 * @return Binary circular kernel
 */
cv::Mat create_circle_kernel(int kernel_size);

/**
 * Apply morphological opening to smooth edges for specified target classes.
 * 
 * @param array Input classification array
 * @param smooth_edge_size Size of circular kernel
 * @param target_class_values Classes to apply edge smoothing to
 * @param background_class_values Classes to preserve as-is
 * @return Smoothed array
 */
cv::Mat smooth_edges(
    const cv::Mat& array,
    int smooth_edge_size,
    const std::vector<int>& target_class_values,
    const std::vector<int>& background_class_values
);

/**
 * Detect small connected components for each target class.
 * 
 * @param smoothed_labels Array after edge smoothing
 * @param target_class_values Classes to analyse
 * @param min_island_size Minimum size threshold
 * @param connectivity Connectivity (4 or 8)
 * @return Map of class values to boolean masks of small islands
 */
std::map<int, cv::Mat> find_small_islands(
    const cv::Mat& smoothed_labels,
    const std::vector<int>& target_class_values,
    int min_island_size,
    int connectivity
);

/**
 * Create combined mask of all pixels requiring gap filling.
 * 
 * @param smoothed_labels Array after edge smoothing
 * @param small_islands_by_class Dictionary of small island masks
 * @return Boolean mask indicating pixels requiring filling
 */
cv::Mat build_invalid_mask(
    const cv::Mat& smoothed_labels,
    const std::map<int, cv::Mat>& small_islands_by_class
);

/**
 * Fill invalid pixels using nearest-neighbour interpolation.
 * 
 * @param smoothed_labels Array with invalid regions
 * @param invalid_mask Boolean mask indicating pixels requiring filling
 * @param all_class_values All valid class values
 * @return Array with invalid regions filled
 */
cv::Mat fill_invalids(
    const cv::Mat& smoothed_labels,
    const cv::Mat& invalid_mask,
    const std::vector<int>& all_class_values
);

} // namespace utils_cpp
