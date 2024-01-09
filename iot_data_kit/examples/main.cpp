// main.cpp
#include "IoTData.h"
#include <iostream>

int main() {
    try {
        // Example data
        std::vector<double> initialData = {10.2, 12.5, 9.8, 11.4, 13.1};

        // Create an IoTData object
        IoTData iotData(initialData);

        // Append new data
        iotData.appendData(14.3);
        iotData.appendData(8.7);

        // Display the data
        std::cout << "Original Data: ";
        iotData.plotData();

        // Filter outliers
        iotData.filterOutliers(2.0);

        // Display filtered data
        std::cout << "Filtered Data: ";
        iotData.plotData();

        // Calculate mean and standard deviation
        std::cout << "Mean: " << iotData.calculateMean() << std::endl;
        std::cout << "Standard Deviation: " << iotData.calculateStandardDeviation() << std::endl;

        // Scale and normalize data
        iotData.scaleData(2.0);
        iotData.normalizeData();

        // Display transformed data
        std::cout << "Transformed Data: ";
        iotData.plotData();

        // Export data to a file
        iotData.exportDataToFile("exported_data.txt");
        std::cout << "Data exported to 'exported_data.txt'" << std::endl;

        // Clear data
        iotData.clearData();

        // Import data from a file
        iotData.importDataFromFile("imported_data.txt");
        std::cout << "Data imported from 'imported_data.txt'" << std::endl;

        // Display imported data
        std::cout << "Imported Data: ";
        iotData.plotData();

        // Test case to validate the fix for standard deviation calculation issue
        IoTData testIoTData({1.0, -2.0, 3.0, -4.0, 5.0});
        std::cout << "Standard Deviation: " << testIoTData.calculateStandardDeviation() << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
