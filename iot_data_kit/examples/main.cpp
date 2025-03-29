// examples/main.cpp
#include "IoTData.h" // Include the header
#include <iostream>
#include <vector>
#include <chrono>
#include <thread> // For std::this_thread::sleep_for
#include <limits> // For numeric_limits

// Helper function to print vectors
template <typename T>
void printVector(const std::string& label, const std::vector<T>& vec) {
    std::cout << label << ": [";
    bool first = true;
    for (const auto& val : vec) {
        if (!first) std::cout << ", ";
        std::cout << val;
        first = false;
    }
    std::cout << "]" << std::endl;
}

// Helper function to print time_points (as epoch seconds for simplicity)
void printTimeVector(const std::string& label, const std::vector<Timestamp>& vec) {
    std::cout << label << ": [";
    bool first = true;
    for (const auto& ts : vec) {
        if (!first) std::cout << ", ";
        std::cout << std::chrono::duration_cast<std::chrono::seconds>(ts.time_since_epoch()).count();
        first = false;
    }
    std::cout << "]" << std::endl;
}

int main() {
    try {
        // Use system_clock for timestamps
        using namespace std::chrono;

        // --- Example with Double Data ---
        std::cout << "--- Double Data Example ---" << std::endl;
        std::vector<double> initialData = {10.2, 12.5, 9.8, 11.4, 13.1, -50.0 /*outlier*/};
        std::vector<Timestamp> initialTimestamps;
        Timestamp now = system_clock::now();
        for (size_t i = 0; i < initialData.size(); ++i) {
            initialTimestamps.push_back(now + seconds(i * 10)); // Timestamps 10 seconds apart
        }

        // Create an IoTData object for doubles
        IoTData<double> iotDataDouble(initialData, initialTimestamps);
        std::cout << "Initial Size: " << iotDataDouble.getDataSize() << std::endl;

        // Append new data
        std::this_thread::sleep_for(seconds(1)); // Ensure timestamp changes
        iotDataDouble.appendData(14.3, system_clock::now());
        std::this_thread::sleep_for(seconds(1));
        iotDataDouble.appendData(8.7, system_clock::now());

        // Display the data (using getData and getTimestamps)
        printVector("Original Data (double)", iotDataDouble.getData());
        printTimeVector("Timestamps (double)", iotDataDouble.getTimestamps());

        // Filter outliers (using a simple threshold here)
        // A more robust method might use standard deviations, IQR, etc.
        double outlierThreshold = 30.0;
        std::cout << "Filtering outliers beyond +/- " << outlierThreshold << std::endl;
        iotDataDouble.filterOutliers([outlierThreshold](double val){
             return std::abs(val) > outlierThreshold;
        });
        printVector("Filtered Data (double)", iotDataDouble.getData());
        printTimeVector("Filtered Timestamps (double)", iotDataDouble.getTimestamps());


        // Calculate statistics
        std::cout << "Mean: " << iotDataDouble.calculateMean() << std::endl;
        std::cout << "Standard Deviation: " << iotDataDouble.calculateStandardDeviation() << std::endl;
        std::cout << "Min: " << iotDataDouble.min() << std::endl;
        std::cout << "Max: " << iotDataDouble.max() << std::endl;
        std::cout << "Median: " << iotDataDouble.median() << std::endl;

        // Calculate Moving Average
        size_t windowSize = 3;
        std::vector<double> movingAvg = iotDataDouble.calculateMovingAverage(windowSize);
        printVector("Moving Average (window=" + std::to_string(windowSize) + ")", movingAvg);

        // Calculate Windowed Mean
        std::vector<double> windowedMean = iotDataDouble.calculateWindowedMean(windowSize);
        printVector("Windowed Mean (window=" + std::to_string(windowSize) + ")", windowedMean);


        // Scale and normalize data
        iotDataDouble.scaleData(2.0);
        printVector("Scaled Data", iotDataDouble.getData());
        iotDataDouble.normalizeData();
        printVector("Normalized Data", iotDataDouble.getData());


        // Export data to a file
        const std::string exportFilename = "exported_data_double.csv";
        iotDataDouble.exportDataToFile(exportFilename);
        std::cout << "Data exported to '" << exportFilename << "'" << std::endl;

        // Clear data
        iotDataDouble.clearData();
        std::cout << "Data cleared. Size: " << iotDataDouble.getDataSize() << std::endl;

        // Import data from a file
        const std::string importFilename = "exported_data_double.csv"; // Use the file we just exported
        std::cout << "Importing data from '" << importFilename << "'" << std::endl;
        iotDataDouble.importDataFromFile(importFilename);
        std::cout << "Data imported. Size: " << iotDataDouble.getDataSize() << std::endl;
        printVector("Imported Data (double)", iotDataDouble.getData());
        printTimeVector("Imported Timestamps (double)", iotDataDouble.getTimestamps());

        // Interpolation example
        std::vector<Timestamp> interpTimestamps;
        if (!iotDataDouble.getTimestamps().empty()) {
             Timestamp start = iotDataDouble.getTimestamps().front() + seconds(5); // Start 5s after first point
             Timestamp end = iotDataDouble.getTimestamps().back() - seconds(5);   // End 5s before last point
             if (start < end) {
                 for (int i = 0; i < 5; ++i) { // Create 5 interpolation points
                    auto duration = duration_cast<seconds>(end - start);
                    interpTimestamps.push_back(start + seconds(duration.count() * i / 4));
                 }
                 printTimeVector("Interpolation Timestamps", interpTimestamps);

                 std::vector<double> linearInterpolated = iotDataDouble.interpolateData(interpTimestamps, InterpolationMethod::LINEAR);
                 printVector("Linearly Interpolated Data", linearInterpolated);

                 // std::vector<double> nearestInterpolated = iotDataDouble.interpolateData(interpTimestamps, InterpolationMethod::NEAREST_NEIGHBOR);
                 // printVector("Nearest Neighbor Interpolated Data", nearestInterpolated);

                 // Note: Cubic spline requires more points and careful implementation, skipping in basic example
             } else {
                 std::cout << "Not enough time range for interpolation example." << std::endl;
             }
        } else {
            std::cout << "Not enough data for interpolation example." << std::endl;
        }


        // --- Example with Integer Data ---
        std::cout << "\n--- Integer Data Example ---" << std::endl;
        std::vector<int> intData = {10, 12, 9, 11, 13, 5};
        std::vector<Timestamp> intTimestamps;
        now = system_clock::now();
        for (size_t i = 0; i < intData.size(); ++i) {
            intTimestamps.push_back(now + minutes(i)); // Timestamps 1 minute apart
        }

        IoTData<int> iotDataInt(intData, intTimestamps);
        printVector("Original Data (int)", iotDataInt.getData());
        printTimeVector("Timestamps (int)", iotDataInt.getTimestamps());

        std::cout << "Mean (int): " << iotDataInt.calculateMean() << std::endl; // Note: Result is double
        std::cout << "Min (int): " << iotDataInt.min() << std::endl;
        std::cout << "Max (int): " << iotDataInt.max() << std::endl;
        std::cout << "Median (int): " << iotDataInt.median() << std::endl; // Note: Result is double if size is even


    } catch (const IoTDataException& e) {
        std::cerr << "IoTData Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Standard Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
