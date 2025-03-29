// include/DataSet.h
#ifndef DATA_SET_H
#define DATA_SET_H

#include <map>
#include <string>
#include <vector>
#include <stdexcept> // For std::runtime_error used by nlohmann::json
#include <fstream> // For file operations

#include "IoTData.h" // Include base class definition
#include "IoTDataException.h" // Custom exceptions
#include "nlohmann/json.hpp" // JSON library

// Convenience alias for the JSON library namespace
using json = nlohmann::json;

/**
 * @brief Template class representing a collection of named IoTData series.
 * Typically used to group related time-series data (e.g., multiple sensors from one device).
 * All series within a DataSet are expected to have the same data type T.
 * @tparam T The numeric type of the data values used in the contained IoTData series.
 */
template <typename T>
class DataSet {
    // Ensure T is a numeric type, consistent with IoTData
    static_assert(std::is_arithmetic_v<T>, "DataSet requires a numeric data type for its series.");

private:
    std::map<std::string, IoTData<T>> seriesMap;

public:
    // --- Constructor ---

    /**
     * @brief Default constructor. Creates an empty DataSet.
     */
    DataSet() = default;

    // --- Series Management ---

    /**
     * @brief Adds an IoTData series to the DataSet.
     * @param name The unique name for this series (e.g., "temperature").
     * @param series The IoTData<T> object to add. Makes a copy.
     * @throws IoTDataException if a series with the same name already exists.
     */
    void addSeries(const std::string& name, const IoTData<T>& series) {
        if (seriesMap.count(name)) {
            throw IoTDataException("Error: Series with name '" + name + "' already exists in the DataSet.");
        }
        seriesMap.emplace(name, series);
        // Alternative: Allow overwrite?
        // seriesMap[name] = series;
    }

    /**
     * @brief Adds an IoTData series to the DataSet by moving it.
     * More efficient if the passed series is temporary or no longer needed.
     * @param name The unique name for this series (e.g., "temperature").
     * @param series The IoTData<T> object to move into the set.
     * @throws IoTDataException if a series with the same name already exists.
     */
    void addSeries(const std::string& name, IoTData<T>&& series) {
         if (seriesMap.count(name)) {
             throw IoTDataException("Error: Series with name '" + name + "' already exists in the DataSet.");
         }
         seriesMap.emplace(name, std::move(series));
    }


    /**
     * @brief Retrieves a const reference to a series by its name.
     * @param name The name of the series to retrieve.
     * @return A const reference to the IoTData<T> object.
     * @throws IoTDataException if no series with the given name exists.
     */
    const IoTData<T>& getSeries(const std::string& name) const {
        auto it = seriesMap.find(name);
        if (it == seriesMap.end()) {
            throw IoTDataException("Error: Series with name '" + name + "' not found in the DataSet.");
        }
        return it->second;
    }

    /**
     * @brief Retrieves a mutable reference to a series by its name.
     * @param name The name of the series to retrieve.
     * @return A reference to the IoTData<T> object.
     * @throws IoTDataException if no series with the given name exists.
     */
    IoTData<T>& getSeries(const std::string& name) {
        auto it = seriesMap.find(name);
        if (it == seriesMap.end()) {
            throw IoTDataException("Error: Series with name '" + name + "' not found in the DataSet.");
        }
        return it->second;
    }

    /**
     * @brief Removes a series from the DataSet by its name.
     * @param name The name of the series to remove.
     * @throws IoTDataException if no series with the given name exists.
     */
    void removeSeries(const std::string& name) {
        if (seriesMap.erase(name) == 0) { // erase returns number of elements removed
            throw IoTDataException("Error: Cannot remove series. Name '" + name + "' not found in the DataSet.");
        }
    }

    /**
     * @brief Checks if a series with the given name exists in the DataSet.
     * @param name The name to check.
     * @return True if the series exists, false otherwise.
     */
    bool hasSeries(const std::string& name) const {
        return seriesMap.count(name) > 0;
    }

    /**
     * @brief Gets a list of names of all series contained in the DataSet.
     * @return A vector of strings containing the series names.
     */
    std::vector<std::string> getSeriesNames() const {
        std::vector<std::string> names;
        names.reserve(seriesMap.size());
        for (const auto& pair : seriesMap) {
            names.push_back(pair.first);
        }
        return names;
    }

    /**
     * @brief Gets the number of series stored in the DataSet.
     * @return The number of series.
     */
    size_t size() const {
        return seriesMap.size();
    }

    /**
     * @brief Checks if the DataSet contains any series.
     * @return True if the DataSet is empty, false otherwise.
     */
    bool empty() const {
        return seriesMap.empty();
    }

    /**
     * @brief Clears all series from the DataSet.
     */
    void clear() {
        seriesMap.clear();
    }

    // --- JSON Import/Export ---

    /**
     * @brief Exports the entire DataSet to a JSON file.
     * Format: {"series_name": {"timestamps_epoch_s": [...], "values": [...]}, ...}
     * @param filename The path to the output JSON file.
     * @throws IoTDataFileException on file opening errors.
     * @throws std::exception (potentially json::exception) on JSON serialization errors.
     */
    void exportToJson(const std::string& filename) const {
        json rootJson = json::object(); // Create root JSON object

        for (const auto& pair : seriesMap) {
            const std::string& name = pair.first;
            const IoTData<T>& series = pair.second;

            json seriesJson = json::object();
            json timestampsJson = json::array();
            json valuesJson = json::array();

            const auto& timestamps = series.getTimestamps();
            const auto& values = series.getData();

            if (timestamps.size() != values.size()) {
                 // Should not happen with valid IoTData objects
                 throw IoTDataException("Internal Error: Mismatch between timestamps and values in series '" + name + "' during JSON export.");
            }

            for (const auto& ts : timestamps) {
                timestampsJson.push_back(std::chrono::duration_cast<std::chrono::seconds>(ts.time_since_epoch()).count());
            }
            for (const auto& val : values) {
                valuesJson.push_back(val);
            }

            seriesJson["timestamps_epoch_s"] = timestampsJson;
            seriesJson["values"] = valuesJson;

            rootJson[name] = seriesJson; // Add the series object to the root object
        }

        // Write JSON to file
        std::ofstream outFile(filename);
        if (!outFile.is_open()) {
            throw IoTDataFileException("Error: Unable to open file '" + filename + "' for DataSet JSON export.");
        }

        // Write with indentation for readability (optional)
        outFile << rootJson.dump(4); // Use dump() for string output, 4 spaces indentation
        outFile.close();
    }


    /**
     * @brief Imports data from a JSON file into the DataSet.
     * Clears any existing data in the DataSet before import.
     * Expected Format: {"series_name": {"timestamps_epoch_s": [...], "values": [...]}, ...}
     * @param filename The path to the input JSON file.
     * @throws IoTDataFileException on file opening or parsing errors.
     * @throws IoTDataException on data format/consistency errors within the JSON.
     * @throws std::exception (potentially json::exception) on general JSON parsing errors.
     */
    void importFromJson(const std::string& filename) {
        // Open and read the file
        std::ifstream inFile(filename);
        if (!inFile.is_open()) {
            throw IoTDataFileException("Error: Unable to open file '" + filename + "' for DataSet JSON import.");
        }

        json rootJson;
        try {
            inFile >> rootJson; // Parse the JSON file
        } catch (const json::parse_error& e) {
            inFile.close();
            throw IoTDataFileException("Error parsing JSON file '" + filename + "': " + e.what());
        }
        inFile.close();

        // Clear existing data
        clear();

        // Check if the root is an object
        if (!rootJson.is_object()) {
            throw IoTDataException("Error: Root element in JSON file '" + filename + "' is not an object.");
        }

        // Iterate through each series in the JSON object
        for (auto it = rootJson.begin(); it != rootJson.end(); ++it) {
            const std::string& seriesName = it.key();
            const json& seriesJson = it.value();

            if (!seriesJson.is_object()) {
                throw IoTDataException("Error: Element for series '" + seriesName + "' in JSON file '" + filename + "' is not an object.");
            }

            // Check for required keys
            if (!seriesJson.contains("timestamps_epoch_s") || !seriesJson.contains("values")) {
                 throw IoTDataException("Error: Series '" + seriesName + "' in JSON file '" + filename + "' is missing 'timestamps_epoch_s' or 'values' key.");
            }

            const json& timestampsJson = seriesJson["timestamps_epoch_s"];
            const json& valuesJson = seriesJson["values"];

            if (!timestampsJson.is_array() || !valuesJson.is_array()) {
                throw IoTDataException("Error: 'timestamps_epoch_s' or 'values' for series '" + seriesName + "' in JSON file '" + filename + "' are not arrays.");
            }

            if (timestampsJson.size() != valuesJson.size()) {
                throw IoTDataException("Error: Mismatch between number of timestamps (" + std::to_string(timestampsJson.size()) +
                                       ") and values (" + std::to_string(valuesJson.size()) + ") for series '" + seriesName +
                                       "' in JSON file '" + filename + "'.");
            }

            std::vector<Timestamp> importedTimestamps;
            std::vector<T> importedValues;
            importedTimestamps.reserve(timestampsJson.size());
            importedValues.reserve(valuesJson.size());

            try {
                for (const auto& tsVal : timestampsJson) {
                     if (!tsVal.is_number_integer()) { // Expecting epoch seconds as integer
                         throw IoTDataException("Error: Non-integer timestamp found for series '" + seriesName + "'.");
                     }
                    long long epoch_s = tsVal.get<long long>();
                    importedTimestamps.push_back(std::chrono::system_clock::from_time_t(static_cast<time_t>(epoch_s)));
                }
                 for (const auto& val : valuesJson) {
                     // Use json's get<T>() which handles type conversion/checking
                     importedValues.push_back(val.get<T>());
                 }
            } catch (const json::type_error& e) {
                 throw IoTDataException("Error processing data for series '" + seriesName + "' in JSON file '" + filename + "': Type mismatch - " + e.what());
            } catch (const IoTDataException& e) { // Catch our specific timestamp type error
                 throw; // Re-throw
            } catch (...) { // Catch any other potential errors during get<>
                 throw IoTDataException("Error processing data for series '" + seriesName + "' in JSON file '" + filename + "': Unknown data conversion error.");
            }


            // Create the IoTData series (constructor handles sorting)
            IoTData<T> newSeries(importedValues, importedTimestamps);

            // Add to the map (using emplace directly might be slightly better than addSeries here)
             seriesMap.emplace(seriesName, std::move(newSeries));
        }
    }

}; // End class DataSet

#endif // DATA_SET_H
