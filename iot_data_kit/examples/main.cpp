// examples/main.cpp
#include "IoTData.h" // Include the header for single series
#include "DataSet.h" // Include the header for data sets
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

        // =====================================================
        // --- Single Series Example (IoTData<T>) ---
        // =====================================================
        std::cout << "--- IoTData<double> Example ---" << std::endl;
        std::vector<double> tempData = {20.5, 21.0, 21.5, 20.8, 22.0};
        std::vector<Timestamp> tempTimestamps;
        Timestamp now = system_clock::now();
        for (size_t i = 0; i < tempData.size(); ++i) {
            tempTimestamps.push_back(now + seconds(i * 5)); // 5 seconds apart
        }
        IoTData<double> temperatureSeries(tempData, tempTimestamps);
        printVector("Temperature Data", temperatureSeries.getData());
        printTimeVector("Temperature Timestamps", temperatureSeries.getTimestamps());
        std::cout << "Temperature Mean: " << temperatureSeries.calculateMean() << std::endl;

        // =====================================================
        // --- Multi-Series Example (DataSet<T>) ---
        // =====================================================
        std::cout << "\n--- DataSet<double> Example ---" << std::endl;

        // Create another series (e.g., humidity)
        std::vector<double> humidData = {60.1, 62.3, 61.5};
        std::vector<Timestamp> humidTimestamps;
        // Use slightly different timestamps for humidity
        for (size_t i = 0; i < humidData.size(); ++i) {
            humidTimestamps.push_back(now + seconds(i * 10 + 2)); // 10 seconds apart, offset by 2s
        }
        IoTData<double> humiditySeries(humidData, humidTimestamps);

        // Create a DataSet
        DataSet<double> sensorDataSet;

        // Add the series to the DataSet
        sensorDataSet.addSeries("temperature", temperatureSeries);
        sensorDataSet.addSeries("humidity", humiditySeries);
        std::cout << "DataSet created with series: ";
        printVector("", sensorDataSet.getSeriesNames());
        std::cout << "DataSet size: " << sensorDataSet.size() << std::endl;

        // Retrieve a series
        if (sensorDataSet.hasSeries("temperature")) {
            const auto& retrievedTemp = sensorDataSet.getSeries("temperature");
            std::cout << "Retrieved Temperature Mean: " << retrievedTemp.calculateMean() << std::endl;
        }

        // Try adding a duplicate name (should throw)
        try {
            sensorDataSet.addSeries("temperature", IoTData<double>({0.0}));
        } catch (const IoTDataException& e) {
            std::cout << "Caught expected exception when adding duplicate series: " << e.what() << std::endl;
        }

        // Export DataSet to JSON
        const std::string jsonFilename = "sensor_data.json";
        std::cout << "Exporting DataSet to '" << jsonFilename << "'..." << std::endl;
        sensorDataSet.exportToJson(jsonFilename);
        std::cout << "Export complete." << std::endl;

        // Create a new DataSet and import from JSON
        DataSet<double> importedDataSet;
        std::cout << "Importing DataSet from '" << jsonFilename << "'..." << std::endl;
        importedDataSet.importFromJson(jsonFilename);
        std::cout << "Import complete." << std::endl;
        std::cout << "Imported DataSet has series: ";
        printVector("", importedDataSet.getSeriesNames());

        // Verify imported data
        if (importedDataSet.hasSeries("humidity")) {
            const auto& retrievedHumid = importedDataSet.getSeries("humidity");
            std::cout << "Imported Humidity Size: " << retrievedHumid.getDataSize() << std::endl;
            printVector("Imported Humidity Data", retrievedHumid.getData());
            // Compare original vs imported (account for epoch second conversion)
             ASSERT_EQ(humiditySeries.getDataSize(), retrievedHumid.getDataSize());
             ASSERT_EQ(humiditySeries.getData(), retrievedHumid.getData());
             for(size_t i=0; i < humiditySeries.getDataSize(); ++i) {
                 ASSERT_EQ(duration_cast<seconds>(humiditySeries.getTimestamps()[i].time_since_epoch()),
                           duration_cast<seconds>(retrievedHumid.getTimestamps()[i].time_since_epoch()));
             }
            std::cout << "Humidity data seems consistent after JSON roundtrip." << std::endl;
        } else {
            std::cerr << "Error: Imported DataSet missing 'humidity' series!" << std::endl;
        }

        // Remove a series
        std::cout << "Removing 'temperature' series..." << std::endl;
        importedDataSet.removeSeries("temperature");
        std::cout << "DataSet series after removal: ";
        printVector("", importedDataSet.getSeriesNames());


    } catch (const IoTDataException& e) {
        std::cerr << "IoTData/DataSet Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        // Catch potential JSON parsing errors or other standard exceptions
        std::cerr << "Standard Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
