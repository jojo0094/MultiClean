#include "utils_cpp.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <cmath>
#include <limits>

namespace py = pybind11;

namespace utils_cpp {

cv::Mat small_islands_mask_for_class(
    const cv::Mat& image,
    int class_value,
    int min_size,
    int connectivity
) {
    // Create binary mask for the specific class
    cv::Mat class_mask_u8;
    cv::compare(image, cv::Scalar(class_value), class_mask_u8, cv::CMP_EQ);
    
    // Check if mask is empty
    if (cv::countNonZero(class_mask_u8) == 0) {
        return cv::Mat::zeros(image.size(), CV_8U);
    }
    
    // Find connected components
    cv::Mat labels, stats, centroids;
    int num_labels = cv::connectedComponentsWithStats(
        class_mask_u8, labels, stats, centroids, connectivity, CV_32S
    );
    
    // Create mask for small components
    cv::Mat small_component_mask = cv::Mat::zeros(image.size(), CV_8U);
    
    for (int i = 1; i < num_labels; i++) {  // Skip background (0)
        int area = stats.at<int>(i, cv::CC_STAT_AREA);
        if (area < min_size) {
            cv::Mat component_mask = (labels == i);
            small_component_mask.setTo(1, component_mask);
        }
    }
    
    return small_component_mask;
}

cv::Mat create_circle_kernel(int kernel_size) {
    cv::Mat kernel = cv::Mat::zeros(kernel_size, kernel_size, CV_8U);
    
    float kernel_center = (kernel_size - 1) / 2.0f;
    float radius_adjustment = (kernel_size < 3) ? 0.1f : 0.4f;
    float effective_radius = kernel_size / 2.0f - radius_adjustment;
    
    for (int row = 0; row < kernel_size; row++) {
        for (int col = 0; col < kernel_size; col++) {
            float distance = std::sqrt(
                std::pow(col - kernel_center, 2) + 
                std::pow(row - kernel_center, 2)
            );
            
            if (distance <= effective_radius) {
                kernel.at<uint8_t>(row, col) = 1;
            }
        }
    }
    
    return kernel;
}

cv::Mat smooth_edges(
    const cv::Mat& array,
    int smooth_edge_size,
    const std::vector<int>& target_class_values,
    const std::vector<int>& background_class_values
) {
    cv::Mat smoothed_labels = cv::Mat(array.size(), CV_32F, cv::Scalar(NAN));
    
    if (smooth_edge_size > 0) {
        cv::Mat kernel = create_circle_kernel(smooth_edge_size);
        
        // Process each target class
        for (int cv_val : target_class_values) {
            cv::Mat class_mask_u8;
            cv::compare(array, cv::Scalar(cv_val), class_mask_u8, cv::CMP_EQ);
            
            // Apply morphological opening
            cv::Mat opened_u8;
            cv::morphologyEx(class_mask_u8, opened_u8, cv::MORPH_OPEN, kernel);
            
            // Set smoothed labels where opened mask is true
            smoothed_labels.setTo(cv::Scalar(static_cast<float>(cv_val)), opened_u8);
        }
        
        // Fill gaps with background classes
        if (!background_class_values.empty()) {
            for (int bg_val : background_class_values) {
                cv::Mat bg_mask;
                cv::compare(array, cv::Scalar(bg_val), bg_mask, cv::CMP_EQ);
                
                cv::Mat nan_mask;
                cv::compare(smoothed_labels, smoothed_labels, nan_mask, cv::CMP_NE);  // NaN check
                
                cv::Mat fill_mask;
                cv::bitwise_and(bg_mask, nan_mask, fill_mask);
                
                if (cv::countNonZero(fill_mask) > 0) {
                    smoothed_labels.setTo(cv::Scalar(static_cast<float>(bg_val)), fill_mask);
                }
            }
        }
    } else {
        array.convertTo(smoothed_labels, CV_32F);
    }
    
    return smoothed_labels;
}

std::map<int, cv::Mat> find_small_islands(
    const cv::Mat& smoothed_labels,
    const std::vector<int>& target_class_values,
    int min_island_size,
    int connectivity
) {
    std::map<int, cv::Mat> small_islands_by_class;
    
    if (min_island_size <= 0) {
        return small_islands_by_class;
    }
    
    for (int cv_val : target_class_values) {
        cv::Mat mask = small_islands_mask_for_class(
            smoothed_labels, cv_val, min_island_size, connectivity
        );
        small_islands_by_class[cv_val] = mask;
    }
    
    return small_islands_by_class;
}

cv::Mat build_invalid_mask(
    const cv::Mat& smoothed_labels,
    const std::map<int, cv::Mat>& small_islands_by_class
) {
    // Create mask for NaN values
    cv::Mat invalid_mask;
    cv::compare(smoothed_labels, smoothed_labels, invalid_mask, cv::CMP_NE);
    
    // Combine with small island masks
    for (const auto& pair : small_islands_by_class) {
        cv::bitwise_or(invalid_mask, pair.second, invalid_mask);
    }
    
    return invalid_mask;
}

cv::Mat fill_invalids(
    const cv::Mat& smoothed_labels,
    const cv::Mat& invalid_mask,
    const std::vector<int>& all_class_values
) {
    cv::Mat output = smoothed_labels.clone();
    
    // Create valid mask
    cv::Mat valid_mask = cv::Mat::zeros(smoothed_labels.size(), CV_8U);
    for (int cv_val : all_class_values) {
        cv::Mat class_mask;
        cv::compare(smoothed_labels, cv::Scalar(static_cast<float>(cv_val)), class_mask, cv::CMP_EQ);
        cv::bitwise_or(valid_mask, class_mask, valid_mask);
    }
    cv::bitwise_and(valid_mask, ~invalid_mask, valid_mask);
    
    if (cv::countNonZero(valid_mask) > 0) {
        // Distance transform to find nearest valid pixels
        cv::Mat dist;
        cv::Mat inverted_valid_mask = ~valid_mask;
        cv::distanceTransform(inverted_valid_mask, dist, cv::DIST_L2, cv::DIST_MASK_PRECISE);
        
        // For each invalid pixel, find nearest valid pixel
        // This is a simplified version - full implementation would use scipy's return_indices
        // For now, use a nearest neighbor approach with inpainting as approximation
        for (int y = 0; y < smoothed_labels.rows; y++) {
            for (int x = 0; x < smoothed_labels.cols; x++) {
                if (invalid_mask.at<uint8_t>(y, x)) {
                    // Find nearest valid pixel
                    float min_dist = std::numeric_limits<float>::max();
                    int nearest_y = y, nearest_x = x;
                    
                    // Search in expanding neighborhood
                    int search_radius = 50;  // Limit search for performance
                    for (int dy = -search_radius; dy <= search_radius; dy++) {
                        for (int dx = -search_radius; dx <= search_radius; dx++) {
                            int ny = y + dy;
                            int nx = x + dx;
                            
                            if (ny >= 0 && ny < smoothed_labels.rows && 
                                nx >= 0 && nx < smoothed_labels.cols &&
                                valid_mask.at<uint8_t>(ny, nx)) {
                                
                                float dist = std::sqrt(dx*dx + dy*dy);
                                if (dist < min_dist) {
                                    min_dist = dist;
                                    nearest_y = ny;
                                    nearest_x = nx;
                                }
                            }
                        }
                    }
                    
                    output.at<float>(y, x) = smoothed_labels.at<float>(nearest_y, nearest_x);
                }
            }
        }
    }
    
    return output;
}

} // namespace utils_cpp

// Python bindings
PYBIND11_MODULE(utils_cpp, m) {
    m.doc() = "C++ implementation of MultiClean utils";
    
    m.def("small_islands_mask_for_class", [](
        py::array_t<float> image,
        int class_value,
        int min_size,
        int connectivity
    ) {
        auto buf = image.request();
        cv::Mat img(buf.shape[0], buf.shape[1], CV_32F, buf.ptr);
        
        cv::Mat result = utils_cpp::small_islands_mask_for_class(img, class_value, min_size, connectivity);
        
        py::array_t<uint8_t> result_array({result.rows, result.cols});
        auto result_buf = result_array.request();
        std::memcpy(result_buf.ptr, result.data, result.total() * result.elemSize());
        
        return result_array;
    }, "Identify small connected components for a single class");
    
    m.def("create_circle_kernel", [](int kernel_size) {
        cv::Mat kernel = utils_cpp::create_circle_kernel(kernel_size);
        
        py::array_t<uint8_t> result_array({kernel.rows, kernel.cols});
        auto result_buf = result_array.request();
        std::memcpy(result_buf.ptr, kernel.data, kernel.total() * kernel.elemSize());
        
        return result_array;
    }, "Create a circular morphological kernel");
    
    m.def("smooth_edges", [](
        py::array_t<int> array,
        int smooth_edge_size,
        const std::vector<int>& target_class_values,
        const std::vector<int>& background_class_values
    ) {
        auto buf = array.request();
        cv::Mat img(buf.shape[0], buf.shape[1], CV_32S, buf.ptr);
        
        cv::Mat result = utils_cpp::smooth_edges(img, smooth_edge_size, target_class_values, background_class_values);
        
        py::array_t<float> result_array({result.rows, result.cols});
        auto result_buf = result_array.request();
        std::memcpy(result_buf.ptr, result.data, result.total() * result.elemSize());
        
        return result_array;
    }, "Apply morphological opening to smooth edges");
    
    m.def("find_small_islands", [](
        py::array_t<float> smoothed_labels,
        const std::vector<int>& target_class_values,
        int min_island_size,
        int connectivity
    ) {
        auto buf = smoothed_labels.request();
        cv::Mat img(buf.shape[0], buf.shape[1], CV_32F, buf.ptr);
        
        auto result = utils_cpp::find_small_islands(img, target_class_values, min_island_size, connectivity);
        
        py::dict result_dict;
        for (const auto& pair : result) {
            py::array_t<uint8_t> mask_array({pair.second.rows, pair.second.cols});
            auto mask_buf = mask_array.request();
            std::memcpy(mask_buf.ptr, pair.second.data, pair.second.total() * pair.second.elemSize());
            result_dict[py::int_(pair.first)] = mask_array;
        }
        
        return result_dict;
    }, "Detect small connected components for each target class");
    
    m.def("build_invalid_mask", [](
        py::array_t<float> smoothed_labels,
        py::dict small_islands_by_class
    ) {
        auto buf = smoothed_labels.request();
        cv::Mat img(buf.shape[0], buf.shape[1], CV_32F, buf.ptr);
        
        std::map<int, cv::Mat> islands_map;
        for (auto item : small_islands_by_class) {
            int key = item.first.cast<int>();
            auto array = item.second.cast<py::array_t<uint8_t>>();
            auto array_buf = array.request();
            cv::Mat mask(array_buf.shape[0], array_buf.shape[1], CV_8U, array_buf.ptr);
            islands_map[key] = mask.clone();
        }
        
        cv::Mat result = utils_cpp::build_invalid_mask(img, islands_map);
        
        py::array_t<uint8_t> result_array({result.rows, result.cols});
        auto result_buf = result_array.request();
        std::memcpy(result_buf.ptr, result.data, result.total() * result.elemSize());
        
        return result_array;
    }, "Create combined mask of all pixels requiring gap filling");
    
    m.def("fill_invalids", [](
        py::array_t<float> smoothed_labels,
        py::array_t<uint8_t> invalid_mask,
        const std::vector<int>& all_class_values
    ) {
        auto buf = smoothed_labels.request();
        cv::Mat img(buf.shape[0], buf.shape[1], CV_32F, buf.ptr);
        
        auto mask_buf = invalid_mask.request();
        cv::Mat mask(mask_buf.shape[0], mask_buf.shape[1], CV_8U, mask_buf.ptr);
        
        cv::Mat result = utils_cpp::fill_invalids(img, mask, all_class_values);
        
        py::array_t<float> result_array({result.rows, result.cols});
        auto result_buf = result_array.request();
        std::memcpy(result_buf.ptr, result.data, result.total() * result.elemSize());
        
        return result_array;
    }, "Fill invalid pixels using nearest-neighbour interpolation");
}
