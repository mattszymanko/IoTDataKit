// tests/DataSetTests.cpp
#include "gtest/gtest.h"
#include "DataSet.h" // Include the header for the class under test
#include "IoTData.h"
#include "IoTDataException.h"
#include "nlohmann/json.hpp" // Required for direct JSON manipulation in tests if needed

#include <vector>
#include <string>
#include <chrono>
#include <fstream> // For creating temporary test files
#include <filesystem> // For robust path handling (C++17)

using namespace std::chrono;
using json = nlohmann::json;

// --- Test Fixture for DataSet ---
class DataSetTest : public ::testing::Test {
protected:
    const std::string testJsonFile = "dataset_test_temp.json";
    const std::string testJsonNonExistent = "non_existent_dataset.json";

    // Clean up any created files after each test
    void TearDown() override {
        std::filesystem::remove(testJsonFile); // Use filesystem remove
    }

    // Helper to create a dummy JSON file for import tests
    void createTestJsonFile(const std::string& filename, const json& content) {
        std::ofstream outFile(filename);
        outFile << content.dump(4); // Pretty print
        outFile.close();
    }

    // Create some sample IoTData series for reuse
    IoTData<double> createSampleTempSeries() {
        std::vector<double> data = {20.0, 21.5, 22.0};
        std::vector<Timestamp> ts = {
            system_clock::from_time_t(1000),
            system_clock::from_time_t(1010),
            system_clock::from_time_t(1020)
        };
        return IoTData<double>(data, ts);
    }

     IoTData<double> createSampleHumidSeries() {
         std::vector<double> data = {55.5, 56.0};
         std::vector<Timestamp> ts = {
             system_clock::from_time_t(1005), // Different timestamps
             system_clock::from_time_t(1015)
         };
         return IoTData<double>(data, ts);
     }

      IoTData<int> createSampleCountSeries() {
          std::vector<int> data = {1, 2, 4, 8};
          std::vector<Timestamp> ts = {
              system_clock::from_time_t(2000),
              system_clock::from_time_t(2010),
              system_clock::from_time_t(2020),
              system_clock::from_time_t(2030)
          };
          return IoTData<int>(data, ts);
      }
};

// --- Constructor & Basic Management Tests ---

TEST_F(DataSetTest, DefaultConstructor) {
    DataSet<double> ds;
    EXPECT_TRUE(ds.empty());
    EXPECT_EQ(ds.size(), 0);
    EXPECT_TRUE(ds.getSeriesNames().empty());
}

TEST_F(DataSetTest, AddSeriesCopy) {
    DataSet<double> ds;
    IoTData<double> temp = createSampleTempSeries();
    const std::string name = "temperature";

    ASSERT_NO_THROW(ds.addSeries(name, temp));
    EXPECT_FALSE(ds.empty());
    EXPECT_EQ(ds.size(), 1);
    EXPECT_TRUE(ds.hasSeries(name));
    EXPECT_FALSE(ds.hasSeries("humidity"));

    auto names = ds.getSeriesNames();
    ASSERT_EQ(names.size(), 1);
    EXPECT_EQ(names[0], name);

    // Check data retrieval
    const auto& retrieved = ds.getSeries(name);
    EXPECT_EQ(retrieved.getData(), temp.getData()); // Compare content
    EXPECT_EQ(retrieved.getTimestamps(), temp.getTimestamps());
}

TEST_F(DataSetTest, AddSeriesMove) {
    DataSet<double> ds;
    IoTData<double> temp = createSampleTempSeries(); // Create data first
    std::vector<double> originalData = temp.getData(); // Copy data for later check
    const std::string name = "temperature";

    // Move the temp series into the dataset
    ASSERT_NO_THROW(ds.addSeries(name, std::move(temp)));
    EXPECT_FALSE(ds.empty());
    EXPECT_EQ(ds.size(), 1);
    EXPECT_TRUE(ds.hasSeries(name));

    // Check retrieval
    const auto& retrieved = ds.getSeries(name);
    EXPECT_EQ(retrieved.getData(), originalData); // Compare content

    // 'temp' should likely be in a valid but unspecified state (e.g., empty) after move
    // ASSERT_TRUE(temp.getData().empty()); // This assertion depends on the specific move behavior of vector/IoTData
}

TEST_F(DataSetTest, AddMultipleSeries) {
    DataSet<double> ds;
    ASSERT_NO_THROW(ds.addSeries("temp", createSampleTempSeries()));
    ASSERT_NO_THROW(ds.addSeries("humid", createSampleHumidSeries()));

    EXPECT_EQ(ds.size(), 2);
    EXPECT_TRUE(ds.hasSeries("temp"));
    EXPECT_TRUE(ds.hasSeries("humid"));

    auto names = ds.getSeriesNames();
    ASSERT_EQ(names.size(), 2);
    // Order in map might vary, so check presence
    EXPECT_NE(std::find(names.begin(), names.end(), "temp"), names.end());
    EXPECT_NE(std::find(names.begin(), names.end(), "humid"), names.end());
}

TEST_F(DataSetTest, AddDuplicateSeriesName) {
    DataSet<double> ds;
    ds.addSeries("temp", createSampleTempSeries());
    // Try adding another series with the same name
    EXPECT_THROW(ds.addSeries("temp", createSampleHumidSeries()), IoTDataException);
}

TEST_F(DataSetTest, GetSeriesNotFound) {
    DataSet<int> ds;
    EXPECT_THROW(ds.getSeries("nonexistent"), IoTDataException);
    // Const version
    const DataSet<int>& const_ds = ds;
    EXPECT_THROW(const_ds.getSeries("nonexistent"), IoTDataException);
}

TEST_F(DataSetTest, RemoveSeries) {
    DataSet<double> ds;
    ds.addSeries("temp", createSampleTempSeries());
    ds.addSeries("humid", createSampleHumidSeries());
    ASSERT_EQ(ds.size(), 2);

    ASSERT_NO_THROW(ds.removeSeries("temp"));
    EXPECT_EQ(ds.size(), 1);
    EXPECT_FALSE(ds.hasSeries("temp"));
    EXPECT_TRUE(ds.hasSeries("humid"));

    // Remove last one
    ASSERT_NO_THROW(ds.removeSeries("humid"));
    EXPECT_EQ(ds.size(), 0);
    EXPECT_TRUE(ds.empty());
    EXPECT_FALSE(ds.hasSeries("humid"));
}

TEST_F(DataSetTest, RemoveSeriesNotFound) {
    DataSet<double> ds;
    EXPECT_THROW(ds.removeSeries("nonexistent"), IoTDataException);
}

TEST_F(DataSetTest, Clear) {
    DataSet<double> ds;
    ds.addSeries("temp", createSampleTempSeries());
    ds.addSeries("humid", createSampleHumidSeries());
    ASSERT_FALSE(ds.empty());

    ds.clear();
    EXPECT_TRUE(ds.empty());
    EXPECT_EQ(ds.size(), 0);
    EXPECT_FALSE(ds.hasSeries("temp"));
    EXPECT_FALSE(ds.hasSeries("humid"));
}


// --- JSON Import/Export Tests ---

TEST_F(DataSetTest, ExportImportRoundtripDouble) {
    DataSet<double> dsOut;
    auto tempSeries = createSampleTempSeries();
    auto humidSeries = createSampleHumidSeries();
    dsOut.addSeries("temperature", tempSeries);
    dsOut.addSeries("humidity", humidSeries);

    // Export
    ASSERT_NO_THROW(dsOut.exportToJson(testJsonFile));

    // Import
    DataSet<double> dsIn;
    ASSERT_NO_THROW(dsIn.importFromJson(testJsonFile));

    // Verify structure
    ASSERT_EQ(dsIn.size(), dsOut.size());
    ASSERT_TRUE(dsIn.hasSeries("temperature"));
    ASSERT_TRUE(dsIn.hasSeries("humidity"));

    // Verify temperature series content
    const auto& tempIn = dsIn.getSeries("temperature");
    ASSERT_EQ(tempIn.getDataSize(), tempSeries.getDataSize());
    EXPECT_EQ(tempIn.getData(), tempSeries.getData());
    ASSERT_EQ(tempIn.getTimestamps().size(), tempSeries.getTimestamps().size());
    for(size_t i=0; i< tempSeries.getDataSize(); ++i) {
         EXPECT_EQ(duration_cast<seconds>(tempIn.getTimestamps()[i].time_since_epoch()),
                   duration_cast<seconds>(tempSeries.getTimestamps()[i].time_since_epoch()));
    }


    // Verify humidity series content
     const auto& humidIn = dsIn.getSeries("humidity");
     ASSERT_EQ(humidIn.getDataSize(), humidSeries.getDataSize());
     EXPECT_EQ(humidIn.getData(), humidSeries.getData());
     ASSERT_EQ(humidIn.getTimestamps().size(), humidSeries.getTimestamps().size());
     for(size_t i=0; i< humidSeries.getDataSize(); ++i) {
          EXPECT_EQ(duration_cast<seconds>(humidIn.getTimestamps()[i].time_since_epoch()),
                    duration_cast<seconds>(humidSeries.getTimestamps()[i].time_since_epoch()));
     }
}

TEST_F(DataSetTest, ExportImportRoundtripInt) {
    DataSet<int> dsOut;
    auto countSeries = createSampleCountSeries();
    dsOut.addSeries("counts", countSeries);

    // Export
    ASSERT_NO_THROW(dsOut.exportToJson(testJsonFile));

    // Import
    DataSet<int> dsIn;
    ASSERT_NO_THROW(dsIn.importFromJson(testJsonFile));

    // Verify structure
    ASSERT_EQ(dsIn.size(), dsOut.size());
    ASSERT_TRUE(dsIn.hasSeries("counts"));

    // Verify content
     const auto& countIn = dsIn.getSeries("counts");
     ASSERT_EQ(countIn.getDataSize(), countSeries.getDataSize());
     EXPECT_EQ(countIn.getData(), countSeries.getData());
     ASSERT_EQ(countIn.getTimestamps().size(), countSeries.getTimestamps().size());
     for(size_t i=0; i< countSeries.getDataSize(); ++i) {
          EXPECT_EQ(duration_cast<seconds>(countIn.getTimestamps()[i].time_since_epoch()),
                    duration_cast<seconds>(countSeries.getTimestamps()[i].time_since_epoch()));
     }
}


TEST_F(DataSetTest, ExportEmptyDataSet) {
    DataSet<float> ds;
    ASSERT_NO_THROW(ds.exportToJson(testJsonFile));

    // Check file content is empty JSON object "{}"
    std::ifstream inFile(testJsonFile);
    json importedJson;
    inFile >> importedJson;
    inFile.close();
    EXPECT_TRUE(importedJson.is_object());
    EXPECT_TRUE(importedJson.empty());
}

TEST_F(DataSetTest, ImportEmptyJsonFile) {
     // Create file with just "{}"
    createTestJsonFile(testJsonFile, json::object());
    DataSet<double> ds;
    ASSERT_NO_THROW(ds.importFromJson(testJsonFile));
    EXPECT_TRUE(ds.empty());
}

TEST_F(DataSetTest, ImportJsonFileNotObject) {
    // Create file with just "[]" (array)
    createTestJsonFile(testJsonFile, json::array());
    DataSet<double> ds;
    EXPECT_THROW(ds.importFromJson(testJsonFile), IoTDataException); // Specific exception expected
}

TEST_F(DataSetTest, ImportJsonFileNotFound) {
    DataSet<double> ds;
    EXPECT_THROW(ds.importFromJson(testJsonNonExistent), IoTDataFileException);
}

TEST_F(DataSetTest, ImportInvalidJsonSyntax) {
    std::ofstream outFile(testJsonFile);
    outFile << "{ \"series\": [1, 2"; // Invalid JSON
    outFile.close();
    DataSet<double> ds;
    EXPECT_THROW(ds.importFromJson(testJsonFile), IoTDataFileException); // Parse error wrapped in FileException
}

TEST_F(DataSetTest, ImportMissingKeys) {
    json badJson = {
        {"temp", { // Missing 'values'
            {"timestamps_epoch_s", {1000, 1010}}
        }}
    };
    createTestJsonFile(testJsonFile, badJson);
    DataSet<double> ds;
    EXPECT_THROW(ds.importFromJson(testJsonFile), IoTDataException);
}

TEST_F(DataSetTest, ImportIncorrectTypes) {
     json badJson = {
         {"temp", {
             {"timestamps_epoch_s", {"not_a_number", 1010}}, // String timestamp
             {"values", {20.0, 21.5}}
         }}
     };
     createTestJsonFile(testJsonFile, badJson);
     DataSet<double> ds;
     EXPECT_THROW(ds.importFromJson(testJsonFile), IoTDataException); // Type error during parsing

     json badJson2 = {
          {"temp", {
              {"timestamps_epoch_s", {1000, 1010}},
              {"values", {"string_val", 21.5}} // String value for double series
          }}
      };
      createTestJsonFile(testJsonFile, badJson2);
      EXPECT_THROW(ds.importFromJson(testJsonFile), IoTDataException); // json::type_error wrapped
}

TEST_F(DataSetTest, ImportTimestampValueMismatch) {
     json badJson = {
         {"temp", {
             {"timestamps_epoch_s", {1000, 1010, 1020}}, // 3 timestamps
             {"values", {20.0, 21.5}}                   // 2 values
         }}
     };
     createTestJsonFile(testJsonFile, badJson);
     DataSet<double> ds;
     EXPECT_THROW(ds.importFromJson(testJsonFile), IoTDataException);
}

// Test case for importing and ensuring data is sorted within each IoTData series
TEST_F(DataSetTest, ImportEnsuresSeriesSorted) {
     json unsortedJson = {
         {"temp", {
             {"timestamps_epoch_s", {1010, 1000, 1020}}, // Unsorted timestamps
             {"values", {21.5, 20.0, 22.0}}             // Corresponding values
         }}
     };
     createTestJsonFile(testJsonFile, unsortedJson);
     DataSet<double> ds;
     ASSERT_NO_THROW(ds.importFromJson(testJsonFile));

     ASSERT_TRUE(ds.hasSeries("temp"));
     const auto& series = ds.getSeries("temp");
     ASSERT_EQ(series.getDataSize(), 3);

     // Check if timestamps are now sorted
     EXPECT_EQ(duration_cast<seconds>(series.getTimestamps()[0].time_since_epoch()).count(), 1000);
     EXPECT_EQ(duration_cast<seconds>(series.getTimestamps()[1].time_since_epoch()).count(), 1010);
     EXPECT_EQ(duration_cast<seconds>(series.getTimestamps()[2].time_since_epoch()).count(), 1020);

     // Check if data matches the sorted timestamp order
     EXPECT_DOUBLE_EQ(series.getData()[0], 20.0);
     EXPECT_DOUBLE_EQ(series.getData()[1], 21.5);
     EXPECT_DOUBLE_EQ(series.getData()[2], 22.0);
}
