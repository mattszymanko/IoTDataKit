// include/IoTData.h
#ifndef IOT_DATA_H
#define IOT_DATA_H

#include <vector>
#include <string>
#include <chrono> // For time points
#include <numeric> // For accumulate, iota
#include <algorithm> // For transform, sort, min_element, max_element, remove_if
#include <cmath> // For abs, sqrt, pow, floor, ceil, isnan, isinf
#include <fstream> // For file operations
#include <sstream> // For string stream parsing
#include <stdexcept> // For standard exceptions
#include <limits> // For numeric_limits
#include <type_traits> // For static_assert, is_arithmetic
#include <functional> // For function objects (like in filterOutliers)
#include <iostream> // For plotData placeholder

#include "IoTDataException.h" // Custom exceptions

// Define Timestamp type alias for clarity
using Timestamp = std::chrono::time_point<std::chrono::system_clock>;

// Enum for interpolation methods
enum class InterpolationMethod {
    LINEAR,
    NEAREST_NEIGHBOR,
    // CUBIC_SPLINE // Note: Cubic spline is complex and requires careful implementation/dependencies. Commented out for now.
};

/**
 * @brief Templated class to store, manage, and analyze time-series data.
 * @tparam T The numeric type of the data values (e.g., double, float, int).
 */
template <typename T>
class IoTData {
    // Ensure T is a numeric type
    static_assert(std::is_arithmetic_v<T>, "IoTData requires a numeric data type (int, float, double, etc.).");

private:
    std::vector<T> data;
    std::vector<Timestamp> timestamps;

    /**
     * @brief Ensures data and timestamps are sorted chronologically.
     * Automatically called after operations that might disrupt order (e.g., import).
     */
    void ensureSorted() {
        if (data.size() != timestamps.size()) {
            // This should ideally not happen if constructors and append are used correctly
            throw IoTDataException("Internal Error: Data and timestamp counts differ during sort.");
        }
        if (data.empty()) return;

        // Create pairs of <timestamp, data>
        std::vector<std::pair<Timestamp, T>> pairedData(data.size());
        for (size_t i = 0; i < data.size(); ++i) {
            pairedData[i] = {timestamps[i], data[i]};
        }

        // Sort pairs based on timestamp
        std::sort(pairedData.begin(), pairedData.end(),
                  [](const auto& a, const auto& b) { return a.first < b.first; });

        // Unpack sorted pairs back into vectors
        for (size_t i = 0; i < data.size(); ++i) {
            timestamps[i] = pairedData[i].first;
            data[i] = pairedData[i].second;
        }
    }

    // --- Interpolation Helper Functions ---
    // Note: Cubic spline helpers removed for now due to complexity.

public:
    // --- Constructors ---

    /**
     * @brief Default constructor. Creates an empty IoTData object.
     */
    IoTData() = default;

    /**
     * @brief Constructor with initial data and automatically generated timestamps (0, 1, 2...).
     * @param initialData The initial data points.
     */
    explicit IoTData(const std::vector<T>& initialData) : data(initialData) {
        timestamps.resize(initialData.size());
        // Generate simple timestamps starting from epoch
        Timestamp startTime = std::chrono::system_clock::from_time_t(0);
        for (size_t i = 0; i < initialData.size(); ++i) {
            timestamps[i] = startTime + std::chrono::seconds(i);
        }
        // No need to sort here as timestamps are generated sequentially
    }

    /**
     * @brief Constructor with initial data and corresponding timestamps.
     * @param initialData The initial data points.
     * @param initialTimestamps The timestamps corresponding to each data point.
     * @throws IoTDataException if the sizes of initialData and initialTimestamps do not match.
     */
    IoTData(const std::vector<T>& initialData, const std::vector<Timestamp>& initialTimestamps)
        : data(initialData), timestamps(initialTimestamps) {
        if (data.size() != timestamps.size()) {
            throw IoTDataException("Error: Number of data points (" + std::to_string(data.size()) +
                                   ") and timestamps (" + std::to_string(timestamps.size()) + ") must match.");
        }
        ensureSorted(); // Ensure data is sorted by timestamp on creation
    }

    // --- Basic Data Manipulation ---

    /**
     * @brief Appends a new data point and its timestamp.
     * @param newData The data value to append.
     * @param timestamp The timestamp of the data point.
     */
    void appendData(T newData, Timestamp timestamp) {
        data.push_back(newData);
        timestamps.push_back(timestamp);
        // Optimization: Only sort if the new timestamp is earlier than the last one
        if (timestamps.size() > 1 && timestamp < timestamps[timestamps.size() - 2]) {
            ensureSorted();
        }
    }

    /**
     * @brief Clears all data and timestamps.
     */
    void clearData() {
        data.clear();
        timestamps.clear();
    }

    /**
     * @brief Gets the number of data points.
     * @return The size of the dataset.
     */
    size_t getDataSize() const {
        return data.size();
    }

    /**
     * @brief Gets a copy of the data vector.
     * @return A const reference to the internal data vector.
     */
    const std::vector<T>& getData() const {
        return data;
    }

    /**
     * @brief Gets a copy of the timestamps vector.
     * @return A const reference to the internal timestamps vector.
     */
    const std::vector<Timestamp>& getTimestamps() const {
        return timestamps;
    }

    // --- Data Filtering ---

    /**
     * @brief Filters out data points based on a predicate function.
     * The corresponding timestamps are also removed.
     * @param predicate A function (or lambda) that takes a value of type T and returns true if the element should be removed.
     * Example: `filterOutliers([](double val){ return std::abs(val) > 100.0; });`
     */
    void filterOutliers(const std::function<bool(T)>& predicate) {
         if (data.empty()) return;
         if (data.size() != timestamps.size()) {
             throw IoTDataException("Internal Error: Data and timestamp counts differ before filtering.");
         }

         std::vector<T> filteredData;
         std::vector<Timestamp> filteredTimestamps;
         filteredData.reserve(data.size());
         filteredTimestamps.reserve(timestamps.size());

         for (size_t i = 0; i < data.size(); ++i) {
             if (!predicate(data[i])) { // Keep elements for which predicate is false
                 filteredData.push_back(data[i]);
                 filteredTimestamps.push_back(timestamps[i]);
             }
         }

         // Check if the size changed before swapping to potentially avoid unnecessary allocation/copy
         if (filteredData.size() != data.size()) {
             data = std::move(filteredData);
             timestamps = std::move(filteredTimestamps);
         }
         // Data remains sorted after filtering
    }

    // --- Statistical Analysis ---

    /**
     * @brief Calculates the arithmetic mean (average) of the data.
     * @return The mean value as a double.
     * @throws IoTDataEmptyException if the dataset is empty.
     * @throws IoTDataException if data contains NaN or Inf values.
     */
    double calculateMean() const {
        if (data.empty()) {
            throw IoTDataEmptyException("Error: No data available for mean calculation.");
        }

        double sum = 0.0;
        for (const T& value : data) {
            if (std::isnan(static_cast<double>(value)) || std::isinf(static_cast<double>(value))) {
                 throw IoTDataException("Error: Data contains invalid values (NaN or infinity) during mean calculation.");
             }
             sum += static_cast<double>(value);
        }

        // Check sum itself for overflow leading to infinity
        if (std::isnan(sum) || std::isinf(sum)) {
             throw IoTDataException("Error: Sum of data values resulted in an invalid value (NaN or infinity).");
        }

        return sum / static_cast<double>(data.size());
    }

    /**
     * @brief Calculates the population standard deviation of the data.
     * @return The standard deviation as a double.
     * @throws IoTDataInsufficientException if the dataset has fewer than 1 point (use N).
     * @throws IoTDataException if data contains NaN or Inf values.
     */
    double calculateStandardDeviation() const {
        if (data.empty()) { // Need at least 1 point for population std dev (though often N-1 is used for sample)
            throw IoTDataInsufficientException("Error: Insufficient data (need at least 1 point) for population standard deviation calculation.");
        }
        if (data.size() == 1) {
            return 0.0; // Standard deviation of a single point is 0
        }

        double mean = calculateMean(); // Reuses checks for NaN/Inf within calculateMean
        double sumSquaredDiff = 0.0;

        for (const T& value : data) {
            // Value validity checked in calculateMean already
            double diff = static_cast<double>(value) - mean;
            sumSquaredDiff += diff * diff;
        }

        // Check sum for validity
        if (std::isnan(sumSquaredDiff) || std::isinf(sumSquaredDiff)) {
             throw IoTDataException("Error: Sum of squared differences resulted in an invalid value (NaN or infinity).");
        }

        // Using N for population standard deviation
        return std::sqrt(sumSquaredDiff / static_cast<double>(data.size()));
    }

    /**
     * @brief Finds the minimum value in the dataset.
     * @return The minimum value.
     * @throws IoTDataEmptyException if the dataset is empty.
     */
    T min() const {
        if (data.empty()) {
            throw IoTDataEmptyException("Error: No data available for minimum calculation.");
        }
        // Filter out potential NaNs if T is float/double
        if constexpr (std::is_floating_point_v<T>) {
            T current_min = std::numeric_limits<T>::infinity();
            bool found_valid = false;
            for (const T& val : data) {
                if (!std::isnan(val)) {
                    if (val < current_min) {
                        current_min = val;
                    }
                    found_valid = true;
                }
            }
            if (!found_valid) {
                throw IoTDataException("Error: Data contains only NaN values during minimum calculation.");
            }
            return current_min;
        } else {
            return *std::min_element(data.begin(), data.end());
        }
    }

    /**
     * @brief Finds the maximum value in the dataset.
     * @return The maximum value.
     * @throws IoTDataEmptyException if the dataset is empty.
     */
    T max() const {
        if (data.empty()) {
            throw IoTDataEmptyException("Error: No data available for maximum calculation.");
        }
        // Filter out potential NaNs if T is float/double
        if constexpr (std::is_floating_point_v<T>) {
             T current_max = -std::numeric_limits<T>::infinity();
             bool found_valid = false;
             for (const T& val : data) {
                 if (!std::isnan(val)) {
                     if (val > current_max) {
                         current_max = val;
                     }
                     found_valid = true;
                 }
             }
             if (!found_valid) {
                 throw IoTDataException("Error: Data contains only NaN values during maximum calculation.");
             }
             return current_max;
         } else {
            return *std::max_element(data.begin(), data.end());
         }
    }

    /**
     * @brief Calculates the median value of the dataset.
     * @return The median value as a double (to handle averages for even counts).
     * @throws IoTDataEmptyException if the dataset is empty.
     */
    double median() const {
        if (data.empty()) {
            throw IoTDataEmptyException("Error: No data available for median calculation.");
        }

        std::vector<T> sortedData = data; // Make a copy to sort
        std::sort(sortedData.begin(), sortedData.end());

        size_t n = sortedData.size();
        if (n % 2 != 0) {
            // Odd number of elements: median is the middle element
            return static_cast<double>(sortedData[n / 2]);
        } else {
            // Even number of elements: median is the average of the two middle elements
            T mid1 = sortedData[n / 2 - 1];
            T mid2 = sortedData[n / 2];
            // Check for NaN/Inf before averaging
             if constexpr (std::is_floating_point_v<T>) {
                 if (std::isnan(static_cast<double>(mid1)) || std::isinf(static_cast<double>(mid1)) ||
                     std::isnan(static_cast<double>(mid2)) || std::isinf(static_cast<double>(mid2))) {
                      throw IoTDataException("Error: Cannot calculate median due to NaN or Inf values near the center.");
                 }
             }
            return (static_cast<double>(mid1) + static_cast<double>(mid2)) / 2.0;
        }
    }


    // --- Data Transformation ---

    /**
     * @brief Scales all data points by a given factor.
     * @param scaleFactor The factor to multiply each data point by.
     */
    void scaleData(double scaleFactor) {
        if (std::isnan(scaleFactor) || std::isinf(scaleFactor)) {
            throw IoTDataException("Error: Invalid scale factor (NaN or Inf).");
        }
        std::transform(data.begin(), data.end(), data.begin(),
                       [scaleFactor](T value) -> T {
                           // Perform calculation in double for precision, then cast back
                           return static_cast<T>(static_cast<double>(value) * scaleFactor);
                       });
    }

    /**
     * @brief Normalizes the data (Z-score normalization: subtract mean, divide by std dev).
     * @throws IoTDataInsufficientException if standard deviation is zero or cannot be calculated.
     * @throws IoTDataException if mean/std dev calculation fails (e.g., NaN/Inf).
     */
    void normalizeData() {
        if (data.size() < 2) {
             throw IoTDataInsufficientException("Error: Insufficient data (need at least 2 points) for normalization.");
        }
        double mean = calculateMean();
        double stdev = calculateStandardDeviation();

        if (stdev == 0.0) {
             // Handle case of constant data - normalization is problematic
             // Option 1: Throw exception
              throw IoTDataInsufficientException("Error: Cannot normalize data with zero standard deviation (constant data).");
             // Option 2: Set all values to 0 (sometimes done, but loses info)
             // std::fill(data.begin(), data.end(), static_cast<T>(0));
             // return;
        }

         if (std::isnan(stdev) || std::isinf(stdev)) {
             throw IoTDataException("Error: Invalid standard deviation (NaN or Inf) during normalization.");
         }


        std::transform(data.begin(), data.end(), data.begin(),
                       [mean, stdev](T value) -> T {
                            // Calculate in double, cast back
                           return static_cast<T>((static_cast<double>(value) - mean) / stdev);
                       });
    }

    // --- Data Export/Import ---

    /**
     * @brief Exports data and timestamps to a CSV file.
     * Timestamps are exported as Unix epoch seconds (integer).
     * @param filename The name of the file to export to.
     * @throws IoTDataFileException if the file cannot be opened for writing.
     */
    void exportDataToFile(const std::string& filename) const {
        std::ofstream outputFile(filename);

        if (!outputFile.is_open()) {
            throw IoTDataFileException("Error: Unable to open file '" + filename + "' for data export.");
        }

        // Optional: Write header
        outputFile << "TimestampEpochSeconds,Value\n";

        for (size_t i = 0; i < data.size(); ++i) {
            auto epochSeconds = std::chrono::duration_cast<std::chrono::seconds>(timestamps[i].time_since_epoch()).count();
            outputFile << epochSeconds << "," << data[i] << '\n';
        }

        outputFile.close(); // Good practice, though destructor handles it
    }

    /**
     * @brief Imports data and timestamps from a CSV file.
     * Assumes format: TimestampEpochSeconds,Value (with optional header).
     * Clears existing data before importing. Sorts data by timestamp after import.
     * @param filename The name of the file to import from.
     * @throws IoTDataFileException if the file cannot be opened, is empty, or has an invalid format.
     */
    void importDataFromFile(const std::string& filename) {
        std::ifstream inputFile(filename);

        if (!inputFile.is_open()) {
            throw IoTDataFileException("Error: Unable to open file '" + filename + "' for data import.");
        }

        clearData(); // Clear existing data

        std::string line;
        long long epochSeconds;
        T value;
        char comma;
        int lineNumber = 0;

        // Optional: Skip header line if present
        if (std::getline(inputFile, line)) {
            lineNumber++;
            std::istringstream headerStream(line);
            // Very basic header check - improve if needed
            if (!(headerStream >> epochSeconds >> comma >> value)) {
                 // Assume it was a header if parsing fails non-catastrophically
                 // Reset stream state and try parsing data from the start
                 inputFile.clear();
                 inputFile.seekg(0, std::ios::beg);
                 line.clear(); // Ensure we read the first line again if it wasn't header
                 lineNumber = 0;
             } else {
                 // If the first line *did* parse as data, rewind and process it
                 inputFile.clear();
                 inputFile.seekg(0, std::ios::beg);
                 line.clear();
                 lineNumber = 0;
            }

        }


        while (std::getline(inputFile, line)) {
            lineNumber++;
            std::istringstream lineStream(line);

             // Check for empty lines
            if (line.find_first_not_of(" \t\n\v\f\r") == std::string::npos) {
                continue; // Skip empty or whitespace-only lines
            }


            if (lineStream >> epochSeconds >> comma && comma == ',' && lineStream >> value) {
                // Check if there's extra stuff after the value
                char remaining;
                if (lineStream >> remaining) {
                     inputFile.close();
                     throw IoTDataFileException("Error in file '" + filename + "' line " + std::to_string(lineNumber) + ": Unexpected characters after value.");
                }

                Timestamp ts = std::chrono::system_clock::from_time_t(static_cast<time_t>(epochSeconds));
                data.push_back(value);
                timestamps.push_back(ts);
            } else {
                inputFile.close();
                throw IoTDataFileException("Error in file '" + filename + "' line " + std::to_string(lineNumber) + ": Invalid format. Expected 'TimestampEpochSeconds,Value'.");
            }
        }

        inputFile.close();

        if (data.empty()) {
            // Distinguish between empty file and file with only header/invalid lines
             if (lineNumber == 0)
                throw IoTDataFileException("Error: Input file '" + filename + "' is empty.");
             else
                 throw IoTDataFileException("Error: No valid data found in input file '" + filename + "'. Check format.");
        }

        ensureSorted(); // Sort data by timestamp after importing
    }

    // --- Data Visualization (Placeholder) ---

    /**
     * @brief Placeholder function for data visualization. Currently prints to console.
     */
    void plotData() const {
        std::cout << "Data plot (TimestampEpochSec, Value): [";
        for (size_t i = 0; i < data.size(); ++i) {
             auto epochSeconds = std::chrono::duration_cast<std::chrono::seconds>(timestamps[i].time_since_epoch()).count();
            std::cout << "(" << epochSeconds << ", " << data[i] << ")" << (i == data.size() - 1 ? "" : ", ");
        }
        std::cout << "]" << std::endl;
    }

    // --- Advanced Analysis & Manipulation ---

     /**
     * @brief Calculates the moving average of the data.
     * The resulting vector will be smaller than the original data (size - windowSize + 1).
     * Each point in the result corresponds to the average of a window ending at that point in the original data.
     * @param windowSize The number of data points to include in each average calculation. Must be > 0.
     * @return A vector of doubles containing the moving averages.
     * @throws IoTDataEmptyException if the dataset is empty.
     * @throws IoTDataInsufficientException if windowSize is larger than the data size or less than 1.
     */
    std::vector<double> calculateMovingAverage(size_t windowSize) const {
        if (data.empty()) {
            throw IoTDataEmptyException("Error: No data available for moving average calculation.");
        }
        if (windowSize == 0) {
             throw IoTDataInsufficientException("Error: Moving average window size must be greater than 0.");
        }
        if (windowSize > data.size()) {
            throw IoTDataInsufficientException("Error: Moving average window size (" + std::to_string(windowSize) +
                                               ") cannot be larger than data size (" + std::to_string(data.size()) + ").");
        }

        std::vector<double> movingAverage;
        movingAverage.reserve(data.size() - windowSize + 1);

        double currentSum = 0.0;
        // Calculate sum for the first window
        for (size_t i = 0; i < windowSize; ++i) {
             if (std::isnan(static_cast<double>(data[i])) || std::isinf(static_cast<double>(data[i]))) {
                 throw IoTDataException("Error: Data contains invalid values (NaN or Inf) in the initial moving average window.");
             }
            currentSum += static_cast<double>(data[i]);
        }
        movingAverage.push_back(currentSum / static_cast<double>(windowSize));

        // Slide the window
        for (size_t i = windowSize; i < data.size(); ++i) {
             if (std::isnan(static_cast<double>(data[i])) || std::isinf(static_cast<double>(data[i]))) {
                  throw IoTDataException("Error: Data contains invalid values (NaN or Inf) in a subsequent moving average window.");
             }
             if (std::isnan(static_cast<double>(data[i - windowSize])) || std::isinf(static_cast<double>(data[i-windowSize]))) {
                 // This check might be redundant if the first loop catches NaNs, but safe to have
                 throw IoTDataException("Error: Invalid value (NaN or Inf) encountered when removing from moving average window.");
             }

            currentSum -= static_cast<double>(data[i - windowSize]); // Subtract the element leaving the window
            currentSum += static_cast<double>(data[i]);             // Add the new element entering the window

             if (std::isnan(currentSum) || std::isinf(currentSum)) {
                 throw IoTDataException("Error: Moving average sum became invalid (NaN or Inf).");
             }

            movingAverage.push_back(currentSum / static_cast<double>(windowSize));
        }

        return movingAverage;
    }

    /**
      * @brief Calculates the mean for fixed-size sliding windows.
      * Differs from moving average in that it returns a vector the same size as the input,
      * where initial windows are calculated with fewer elements.
      * @param windowSize The maximum number of data points in the window.
      * @return A vector of doubles containing the windowed means.
      * @throws IoTDataEmptyException if the dataset is empty.
      * @throws IoTDataInsufficientException if windowSize is less than 1.
      */
     std::vector<double> calculateWindowedMean(size_t windowSize) const {
         if (data.empty()) {
             throw IoTDataEmptyException("Error: No data available for windowed mean calculation.");
         }
         if (windowSize == 0) {
              throw IoTDataInsufficientException("Error: Windowed mean size must be greater than 0.");
         }

         std::vector<double> windowedMean;
         windowedMean.reserve(data.size());

         double currentSum = 0.0;

         for (size_t i = 0; i < data.size(); ++i) {
             if (std::isnan(static_cast<double>(data[i])) || std::isinf(static_cast<double>(data[i]))) {
                 throw IoTDataException("Error: Data contains invalid values (NaN or Inf) during windowed mean calculation.");
             }
             currentSum += static_cast<double>(data[i]);

             size_t currentWindowSize;
             if (i >= windowSize) {
                 // Subtract the element that falls out of the window
                 if (std::isnan(static_cast<double>(data[i - windowSize])) || std::isinf(static_cast<double>(data[i - windowSize]))) {
                      throw IoTDataException("Error: Invalid value (NaN or Inf) encountered when removing from windowed mean window.");
                 }
                 currentSum -= static_cast<double>(data[i - windowSize]);
                 currentWindowSize = windowSize;
             } else {
                 // Window is still growing
                 currentWindowSize = i + 1;
             }

              if (std::isnan(currentSum) || std::isinf(currentSum)) {
                  throw IoTDataException("Error: Windowed mean sum became invalid (NaN or Inf).");
              }

             windowedMean.push_back(currentSum / static_cast<double>(currentWindowSize));
         }

         return windowedMean;
     }


    /**
     * @brief Resamples the data to a target size using linear interpolation.
     * Requires at least two data points. Timestamps are evenly spaced in the result.
     * @param targetSize The desired number of data points in the resampled data. Must be >= 2.
     * @return A vector of type T containing the resampled data.
     * @throws IoTDataEmptyException if the dataset is empty.
     * @throws IoTDataInsufficientException if data size is less than 2 or targetSize is less than 2.
     */
    std::vector<T> resampleData(size_t targetSize) const {
        if (data.size() < 2) {
            throw IoTDataInsufficientException("Error: Need at least 2 data points for resampling.");
        }
         if (targetSize < 2) {
            throw IoTDataInsufficientException("Error: Target size for resampling must be at least 2.");
        }

        std::vector<T> resampledData;
        resampledData.reserve(targetSize);

        // Calculate time duration and step for new timestamps
        auto totalDuration = timestamps.back() - timestamps.front();
        // Use double for intermediate duration calculation to avoid potential overflow/precision issues with long durations
        double totalDurationSec = std::chrono::duration<double>(totalDuration).count();
        double timeStepSec = totalDurationSec / static_cast<double>(targetSize - 1);

        // Add the first point directly
        resampledData.push_back(data.front());

        for (size_t i = 1; i < targetSize -1; ++i) {
             // Calculate the target timestamp for this point
             Timestamp targetTs = timestamps.front() + std::chrono::duration<double>(i * timeStepSec);

             // Find the surrounding original timestamps (use lower_bound for efficiency)
             auto it = std::lower_bound(timestamps.begin(), timestamps.end(), targetTs);

             // Handle edge cases: targetTs exactly matches an existing timestamp or is outside range
             if (it == timestamps.end()) { // Should not happen if targetTs <= timestamps.back()
                 resampledData.push_back(data.back());
                 continue;
             }
             if (*it == targetTs) { // Exact match
                 size_t index = std::distance(timestamps.begin(), it);
                 resampledData.push_back(data[index]);
                 continue;
             }
              if (it == timestamps.begin()) { // TargetTs is before the first timestamp (shouldn't happen with calculation)
                  resampledData.push_back(data.front());
                  continue;
              }


             // Linear interpolation
             size_t upperIndex = std::distance(timestamps.begin(), it);
             size_t lowerIndex = upperIndex - 1;

             Timestamp t0 = timestamps[lowerIndex];
             Timestamp t1 = timestamps[upperIndex];
             T y0 = data[lowerIndex];
             T y1 = data[upperIndex];

             // Calculate interpolation factor (avoid division by zero if t1 == t0)
             auto segmentDuration = t1 - t0;
             double segmentDurationSec = std::chrono::duration<double>(segmentDuration).count();
             if (segmentDurationSec == 0.0) {
                 // Should only happen if duplicate timestamps exist, return lower value
                 resampledData.push_back(y0);
             } else {
                 auto targetOffset = targetTs - t0;
                 double targetOffsetSec = std::chrono::duration<double>(targetOffset).count();
                 double factor = targetOffsetSec / segmentDurationSec;

                 // Perform interpolation using double, then cast back to T
                  double interpolatedValue = static_cast<double>(y0) + factor * (static_cast<double>(y1) - static_cast<double>(y0));

                  // Check for NaN/Inf in result (can happen with large value differences)
                  if (std::isnan(interpolatedValue) || std::isinf(interpolatedValue)) {
                      throw IoTDataException("Error: Interpolated value resulted in NaN or Inf during resampling.");
                  }
                 resampledData.push_back(static_cast<T>(interpolatedValue));
             }
        }
         // Add the last point directly
         resampledData.push_back(data.back());

        return resampledData;
    }

    /**
     * @brief Trims a percentage of data from both the beginning and the end.
     * @param trimPercentage The total percentage of data to remove (e.g., 10.0 means 5% from start, 5% from end). Must be between 0 and 100.
     * @throws IoTDataEmptyException if the dataset is empty.
     * @throws std::out_of_range if trimPercentage is not between 0 and 100.
     */
    void trimData(double trimPercentage) {
        if (trimPercentage < 0.0 || trimPercentage >= 100.0) {
            throw std::out_of_range("Error: Trim percentage (" + std::to_string(trimPercentage) + ") must be between 0 and 100 (exclusive of 100).");
        }
        if (data.empty()) {
            // Nothing to trim, not necessarily an error, maybe just return? Or throw?
            // Let's just return silently for empty data.
            return;
            // Or: throw IoTDataEmptyException("Error: No data available for trimming.");
        }


        size_t totalSize = data.size();
        // Calculate points to trim from *each* end
        size_t trimCount = static_cast<size_t>(static_cast<double>(totalSize) * trimPercentage / 200.0);

        if (trimCount * 2 >= totalSize) {
             // Avoid trimming everything, clear instead
             clearData();
        } else if (trimCount > 0) {
            // Erase from beginning
            data.erase(data.begin(), data.begin() + trimCount);
            timestamps.erase(timestamps.begin(), timestamps.begin() + trimCount);
            // Erase from end
            data.erase(data.end() - trimCount, data.end());
            timestamps.erase(timestamps.end() - trimCount, timestamps.end());
        }
        // Data remains sorted after trimming
    }

    // --- Data Interpolation ---

    /**
     * @brief Interpolates data values at new, specified timestamps.
     * Requires the internal data to be sorted by timestamp (ensured by constructors, append, import).
     * @param newTimestamps A vector of timestamps at which to interpolate data values. Must be sorted.
     * @param method The interpolation method to use (LINEAR, NEAREST_NEIGHBOR).
     * @return A vector of type T containing the interpolated data values.
     * @throws IoTDataEmptyException if the dataset is empty.
     * @throws IoTDataInsufficientException for methods requiring multiple points if data is insufficient.
     * @throws IoTDataException if newTimestamps are not sorted or interpolation calculation fails.
     */
    std::vector<T> interpolateData(const std::vector<Timestamp>& newTimestamps,
                                     InterpolationMethod method = InterpolationMethod::LINEAR) const {
        if (data.empty()) {
            throw IoTDataEmptyException("Error: No data available for interpolation.");
        }

        // Check if newTimestamps are sorted
        for (size_t i = 1; i < newTimestamps.size(); ++i) {
            if (newTimestamps[i] < newTimestamps[i-1]) {
                throw IoTDataException("Error: newTimestamps vector must be sorted for interpolation.");
            }
        }


        std::vector<T> interpolatedData;
        interpolatedData.reserve(newTimestamps.size());

        for (const Timestamp& t : newTimestamps) {
            // Find the first timestamp in our data that is not less than t
            auto it = std::lower_bound(timestamps.begin(), timestamps.end(), t);

            // --- Handle boundary conditions ---
            if (it == timestamps.begin()) {
                // t is before or exactly at the first timestamp
                interpolatedData.push_back(data.front());
            } else if (it == timestamps.end()) {
                // t is after or exactly at the last timestamp
                interpolatedData.push_back(data.back());
            } else {
                // t is between two timestamps or exactly matches one
                size_t upperIndex = std::distance(timestamps.begin(), it);
                size_t lowerIndex = upperIndex - 1; // Safe because it != timestamps.begin()

                Timestamp t0 = timestamps[lowerIndex];
                Timestamp t1 = timestamps[upperIndex];
                T y0 = data[lowerIndex];
                T y1 = data[upperIndex];

                // If t exactly matches an existing timestamp
                 if (t == t1) { // Check upper bound first as lower_bound finds >= t
                     interpolatedData.push_back(y1);
                     continue;
                 }
                  if (t == t0) { // Should be caught by the t==t1 check in next iteration's lower_bound unless duplicate timestamps
                      interpolatedData.push_back(y0);
                      continue;
                  }

                // --- Perform interpolation based on method ---
                switch (method) {
                    case InterpolationMethod::LINEAR: {
                         if (t1 == t0) { // Handle duplicate timestamps
                             interpolatedData.push_back(y0); // Or y1, or average? Let's take the earlier one.
                             continue;
                         }
                        // Calculate interpolation factor
                        auto segmentDuration = t1 - t0;
                        auto targetOffset = t - t0;
                        // Use double for factor calculation for precision
                        double factor = std::chrono::duration<double>(targetOffset).count() / std::chrono::duration<double>(segmentDuration).count();

                        // Perform interpolation in double, then cast back
                        double interpolatedValue = static_cast<double>(y0) + factor * (static_cast<double>(y1) - static_cast<double>(y0));

                         if (std::isnan(interpolatedValue) || std::isinf(interpolatedValue)) {
                             throw IoTDataException("Error: Linear interpolation resulted in NaN or Inf.");
                         }

                        interpolatedData.push_back(static_cast<T>(interpolatedValue));
                        break;
                    }

                    case InterpolationMethod::NEAREST_NEIGHBOR: {
                        auto diff_prev = t - t0;
                        auto diff_next = t1 - t; // Note: t1 >= t >= t0
                        interpolatedData.push_back(diff_prev <= diff_next ? y0 : y1);
                        break;
                    }

                    // case InterpolationMethod::CUBIC_SPLINE:
                    //     // Requires more complex setup, potentially external library or more involved implementation.
                    //     throw IoTDataException("Cubic spline interpolation not implemented yet.");
                    //     break;

                    default:
                        // Should not happen if enum is handled correctly
                        throw IoTDataException("Unknown interpolation method specified.");
                }
            }
        } // end for loop over newTimestamps

        return interpolatedData;
    }

}; // End class IoTData

#endif // IOT_DATA_H
