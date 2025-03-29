// tests/IoTDataTests.cpp
#include "gtest/gtest.h"
#include "IoTData.h" // Include the header for the class under test
#include "IoTDataException.h" // Include custom exceptions

#include <vector>
#include <chrono>
#include <cmath> // For std::abs, std::isnan
#include <limits> // For std::numeric_limits
#include <fstream> // For creating temporary test files

using namespace std::chrono;

// --- Test Fixture ---
// Can be used to set up common data for multiple tests
class IoTDataTest : public ::testing::Test {
protected:
    const std::string testInputFile = "test_input.csv";
    const std::string testOutputFile = "test_output.csv";

    // Clean up any created files after each test
    void TearDown() override {
        std::remove(testInputFile.c_str());
        std::remove(testOutputFile.c_str());
    }

    // Helper to create a dummy CSV file for import tests
    void createTestFile(const std::string& filename, const std::vector<std::pair<long long, double>>& data) {
        std::ofstream outFile(filename);
        // outFile << "TimestampEpochSeconds,Value\n"; // Optional header
        for (const auto& p : data) {
            outFile << p.first << "," << p.second << "\n";
        }
        outFile.close();
    }
};


// --- Constructor Tests ---

TEST_F(IoTDataTest, DefaultConstructor) {
    IoTData<double> data;
    EXPECT_EQ(data.getDataSize(), 0);
    EXPECT_TRUE(data.getData().empty());
    EXPECT_TRUE(data.getTimestamps().empty());
}

TEST_F(IoTDataTest, ConstructorWithDataOnly) {
    std::vector<int> initialData = {1, 2, 3};
    IoTData<int> data(initialData);
    EXPECT_EQ(data.getDataSize(), 3);
    EXPECT_EQ(data.getData(), initialData);
    ASSERT_EQ(data.getTimestamps().size(), 3);
    // Check basic sequential timestamps (seconds from epoch 0)
    EXPECT_EQ(duration_cast<seconds>(data.getTimestamps()[0].time_since_epoch()).count(), 0);
    EXPECT_EQ(duration_cast<seconds>(data.getTimestamps()[1].time_since_epoch()).count(), 1);
    EXPECT_EQ(duration_cast<seconds>(data.getTimestamps()[2].time_since_epoch()).count(), 2);
}


TEST_F(IoTDataTest, ConstructorWithDataAndTimestamps) {
    std::vector<double> initialData = {10.1, 20.2};
    std::vector<Timestamp> initialTimestamps = {
        system_clock::now(),
        system_clock::now() + seconds(10)
    };
    IoTData<double> data(initialData, initialTimestamps);
    EXPECT_EQ(data.getDataSize(), 2);
    EXPECT_EQ(data.getData(), initialData);
    EXPECT_EQ(data.getTimestamps(), initialTimestamps);
}

TEST_F(IoTDataTest, ConstructorTimestampDataMismatch) {
    std::vector<double> initialData = {10.1, 20.2, 30.3};
    std::vector<Timestamp> initialTimestamps = {
        system_clock::now(),
        system_clock::now() + seconds(10)
    };
    // Expect exception
    EXPECT_THROW({
        IoTData<double> data(initialData, initialTimestamps);
    }, IoTDataException);
}

TEST_F(IoTDataTest, ConstructorSortsInput) {
     std::vector<float> initialData = {1.0f, 2.0f, 3.0f};
     Timestamp t1 = system_clock::now();
     Timestamp t2 = t1 + seconds(10);
     Timestamp t0 = t1 - seconds(5); // Earlier timestamp
     // Provide timestamps out of order
     std::vector<Timestamp> initialTimestamps = {t1, t2, t0};
     std::vector<float> expectedData = {3.0f, 1.0f, 2.0f}; // Expected order after sort
     std::vector<Timestamp> expectedTimestamps = {t0, t1, t2};

     IoTData<float> data(initialData, initialTimestamps);

     EXPECT_EQ(data.getDataSize(), 3);
     EXPECT_EQ(data.getData(), expectedData);
     EXPECT_EQ(data.getTimestamps(), expectedTimestamps);
}


// --- Basic Manipulation Tests ---

TEST_F(IoTDataTest, AppendData) {
    IoTData<int> data;
    Timestamp t1 = system_clock::now();
    Timestamp t2 = t1 + seconds(5);
    Timestamp t0 = t1 - seconds(2); // Add an earlier timestamp

    data.appendData(10, t1);
    EXPECT_EQ(data.getDataSize(), 1);
    EXPECT_EQ(data.getData().back(), 10);
    EXPECT_EQ(data.getTimestamps().back(), t1);

    data.appendData(20, t2); // Append later timestamp
    EXPECT_EQ(data.getDataSize(), 2);
    EXPECT_EQ(data.getData().back(), 20);
    EXPECT_EQ(data.getTimestamps().back(), t2);
    EXPECT_EQ(data.getData()[0], 10); // Check order maintained
    EXPECT_EQ(data.getTimestamps()[0], t1);

    data.appendData(5, t0); // Append earlier timestamp, should trigger sort
    EXPECT_EQ(data.getDataSize(), 3);
    // Check final sorted order
    EXPECT_EQ(data.getData()[0], 5);
    EXPECT_EQ(data.getTimestamps()[0], t0);
    EXPECT_EQ(data.getData()[1], 10);
    EXPECT_EQ(data.getTimestamps()[1], t1);
    EXPECT_EQ(data.getData()[2], 20);
    EXPECT_EQ(data.getTimestamps()[2], t2);
}

TEST_F(IoTDataTest, ClearData) {
    IoTData<double> data({1.1, 2.2});
    ASSERT_EQ(data.getDataSize(), 2);
    data.clearData();
    EXPECT_EQ(data.getDataSize(), 0);
    EXPECT_TRUE(data.getData().empty());
    EXPECT_TRUE(data.getTimestamps().empty());
}

TEST_F(IoTDataTest, GetDataSize) {
    IoTData<float> data;
    EXPECT_EQ(data.getDataSize(), 0);
    data.appendData(1.0f, system_clock::now());
    EXPECT_EQ(data.getDataSize(), 1);
    data.appendData(2.0f, system_clock::now());
    EXPECT_EQ(data.getDataSize(), 2);
}

TEST_F(IoTDataTest, GetDataAndTimestamps) {
     std::vector<double> initialData = {10.1, 20.2};
     std::vector<Timestamp> initialTimestamps = {
         system_clock::now(),
         system_clock::now() + seconds(10)
     };
     IoTData<double> data(initialData, initialTimestamps);
     const auto& retrievedData = data.getData();
     const auto& retrievedTs = data.getTimestamps();
     EXPECT_EQ(retrievedData, initialData);
     EXPECT_EQ(retrievedTs, initialTimestamps);
}


// --- Statistics Tests ---

TEST_F(IoTDataTest, CalculateMeanEmpty) {
    IoTData<double> data;
    EXPECT_THROW(data.calculateMean(), IoTDataEmptyException);
}

TEST_F(IoTDataTest, CalculateMeanSingle) {
    IoTData<double> data({15.5});
    EXPECT_DOUBLE_EQ(data.calculateMean(), 15.5);
}

TEST_F(IoTDataTest, CalculateMeanMultiple) {
    IoTData<int> data({1, 2, 3, 4, 5}); // Sum = 15, Count = 5
    EXPECT_DOUBLE_EQ(data.calculateMean(), 3.0);

    IoTData<double> data_double({1.0, -2.0, 3.0, -4.0, 5.0}); // Sum = 3.0, Count = 5
    EXPECT_DOUBLE_EQ(data.calculateMean(), 3.0); // Original test case had wrong expected value? Sum = 3, Mean = 0.6
    EXPECT_DOUBLE_EQ(data_double.calculateMean(), 0.6); // Corrected expectation
}

TEST_F(IoTDataTest, CalculateMeanWithNaN) {
    if constexpr (std::numeric_limits<double>::has_quiet_NaN) {
        IoTData<double> data({1.0, 2.0, std::numeric_limits<double>::quiet_NaN()});
        EXPECT_THROW(data.calculateMean(), IoTDataException);
    } else {
        GTEST_SKIP() << "Skipping NaN test as quiet_NaN is not supported.";
    }
}

TEST_F(IoTDataTest, CalculateMeanWithInf) {
     if constexpr (std::numeric_limits<double>::has_infinity) {
        IoTData<double> data({1.0, 2.0, std::numeric_limits<double>::infinity()});
        EXPECT_THROW(data.calculateMean(), IoTDataException);
     } else {
        GTEST_SKIP() << "Skipping Inf test as infinity is not supported.";
     }
}

TEST_F(IoTDataTest, CalculateStdDevInsufficient) {
    IoTData<double> data;
    EXPECT_THROW(data.calculateStandardDeviation(), IoTDataInsufficientException);
    // Population standard deviation requires only 1 point, but returns 0.
    // IoTData<double> data_one({5.0});
    // EXPECT_THROW(data_one.calculateStandardDeviation(), IoTDataInsufficientException);
}

TEST_F(IoTDataTest, CalculateStdDevSinglePoint) {
     IoTData<double> data_one({5.0});
     // Population std dev of one point is 0
     EXPECT_DOUBLE_EQ(data_one.calculateStandardDeviation(), 0.0);
}


TEST_F(IoTDataTest, CalculateStdDevMultiple) {
    // Population Std Dev: sqrt( Sum[(x_i - mean)^2] / N )
    IoTData<int> data({1, 2, 3, 4, 5}); // Mean = 3
    // Diffs: -2, -1, 0, 1, 2
    // Squared Diffs: 4, 1, 0, 1, 4 => Sum = 10
    // Variance = 10 / 5 = 2
    // Std Dev = sqrt(2)
    EXPECT_DOUBLE_EQ(data.calculateStandardDeviation(), std::sqrt(2.0));

    IoTData<double> data_double({2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0}); // N=8, Sum=40, Mean=5
    // Diffs: -3, -1, -1, -1, 0, 0, 2, 4
    // Sq Diffs: 9, 1, 1, 1, 0, 0, 4, 16 => Sum = 32
    // Variance = 32 / 8 = 4
    // Std Dev = sqrt(4) = 2
    EXPECT_DOUBLE_EQ(data_double.calculateStandardDeviation(), 2.0);
}

TEST_F(IoTDataTest, MinMaxEmpty) {
    IoTData<int> data;
    EXPECT_THROW(data.min(), IoTDataEmptyException);
    EXPECT_THROW(data.max(), IoTDataEmptyException);
}

TEST_F(IoTDataTest, MinMax) {
    IoTData<int> data({3, 1, 4, 1, 5, 9, 2, 6});
    EXPECT_EQ(data.min(), 1);
    EXPECT_EQ(data.max(), 9);

    IoTData<double> data_double({-1.5, 0.0, 10.2, -5.0});
     EXPECT_DOUBLE_EQ(data_double.min(), -5.0);
     EXPECT_DOUBLE_EQ(data_double.max(), 10.2);
}

TEST_F(IoTDataTest, MedianEmpty) {
    IoTData<int> data;
    EXPECT_THROW(data.median(), IoTDataEmptyException);
}

TEST_F(IoTDataTest, MedianOdd) {
    IoTData<int> data({3, 1, 4, 5, 2}); // Sorted: 1, 2, 3, 4, 5
    EXPECT_DOUBLE_EQ(data.median(), 3.0);
}

TEST_F(IoTDataTest, MedianEven) {
    IoTData<double> data({3.0, 1.0, 4.0, 5.0, 2.0, 6.0}); // Sorted: 1, 2, 3, 4, 5, 6
    // Median = (3.0 + 4.0) / 2 = 3.5
    EXPECT_DOUBLE_EQ(data.median(), 3.5);
}

// --- File I/O Tests ---

TEST_F(IoTDataTest, ExportImportRoundtrip) {
    std::vector<double> initialData = {10.1, -20.2, 30.3};
    std::vector<Timestamp> initialTimestamps = {
        system_clock::from_time_t(1678886400), // Specific time points
        system_clock::from_time_t(1678886460),
        system_clock::from_time_t(1678886520)
    };
    IoTData<double> dataOut(initialData, initialTimestamps);

    // Export
    ASSERT_NO_THROW(dataOut.exportDataToFile(testOutputFile));

    // Import
    IoTData<double> dataIn;
    ASSERT_NO_THROW(dataIn.importDataFromFile(testOutputFile));

    // Verify
    EXPECT_EQ(dataIn.getDataSize(), initialData.size());
    EXPECT_EQ(dataIn.getData(), initialData); // Data should match exactly
    // Timestamps might have second-level precision from epoch conversion
    ASSERT_EQ(dataIn.getTimestamps().size(), initialTimestamps.size());
    for(size_t i = 0; i < initialTimestamps.size(); ++i) {
        EXPECT_EQ(duration_cast<seconds>(dataIn.getTimestamps()[i].time_since_epoch()),
                    duration_cast<seconds>(initialTimestamps[i].time_since_epoch()));
    }
}

TEST_F(IoTDataTest, ImportFileNotFound) {
    IoTData<double> data;
    EXPECT_THROW(data.importDataFromFile("non_existent_file.csv"), IoTDataFileException);
}

TEST_F(IoTDataTest, ImportEmptyFile) {
    createTestFile(testInputFile, {}); // Create empty file
    IoTData<double> data;
    EXPECT_THROW(data.importDataFromFile(testInputFile), IoTDataFileException);
}

TEST_F(IoTDataTest, ImportInvalidFormat) {
    std::ofstream outFile(testInputFile);
    outFile << "timestamp,value\n"; // Header is ok
    outFile << "1678886400,10.1\n"; // Good line
    outFile << "1678886460;20.2\n"; // Bad delimiter
    outFile << "1678886520,30.3\n"; // Good line
    outFile.close();

    IoTData<double> data;
    EXPECT_THROW(data.importDataFromFile(testInputFile), IoTDataFileException);
}

TEST_F(IoTDataTest, ImportSortsData) {
    // Data with out-of-order timestamps
    createTestFile(testInputFile, {{100, 1.0}, {50, 0.5}, {150, 1.5}});
    IoTData<double> data;
    ASSERT_NO_THROW(data.importDataFromFile(testInputFile));

    ASSERT_EQ(data.getDataSize(), 3);
    // Check if data is sorted by timestamp
    EXPECT_EQ(duration_cast<seconds>(data.getTimestamps()[0].time_since_epoch()).count(), 50);
    EXPECT_DOUBLE_EQ(data.getData()[0], 0.5);
    EXPECT_EQ(duration_cast<seconds>(data.getTimestamps()[1].time_since_epoch()).count(), 100);
    EXPECT_DOUBLE_EQ(data.getData()[1], 1.0);
    EXPECT_EQ(duration_cast<seconds>(data.getTimestamps()[2].time_since_epoch()).count(), 150);
    EXPECT_DOUBLE_EQ(data.getData()[2], 1.5);
}

// --- Add more tests! ---
// Need tests for:
// - filterOutliers (various cases, ensure timestamp sync)
// - scaleData
// - normalizeData (include zero std dev case)
// - calculateMovingAverage (edge cases, window sizes)
// - calculateWindowedMean
// - resampleData
// - trimData (percentages, edge cases)
// - interpolateData (LINEAR, NEAREST, boundary conditions, unsorted newTimestamps)

TEST_F(IoTDataTest, NormalizeDataZeroStdDev) {
    IoTData<double> data({5.0, 5.0, 5.0});
    EXPECT_THROW(data.normalizeData(), IoTDataInsufficientException); // Expecting exception for zero std dev
}

TEST_F(IoTDataTest, InterpolateLinear) {
     std::vector<double> initialData = {0.0, 10.0, 20.0};
     std::vector<Timestamp> initialTimestamps = {
         system_clock::from_time_t(0),
         system_clock::from_time_t(10),
         system_clock::from_time_t(20)
     };
     IoTData<double> data(initialData, initialTimestamps);

     std::vector<Timestamp> newTs = {
         system_clock::from_time_t(5),  // Midpoint 1 -> 5.0
         system_clock::from_time_t(10), // Exact point -> 10.0
         system_clock::from_time_t(18)  // 80% between 10 and 20 -> 10 + 0.8 * (20-10) = 18.0
     };

     std::vector<double> expected = {5.0, 10.0, 18.0};
     std::vector<double> result = data.interpolateData(newTs, InterpolationMethod::LINEAR);

     ASSERT_EQ(result.size(), expected.size());
     for(size_t i=0; i<result.size(); ++i) {
         EXPECT_DOUBLE_EQ(result[i], expected[i]);
     }
}

TEST_F(IoTDataTest, InterpolateNearest) {
      std::vector<double> initialData = {0.0, 10.0, 20.0};
      std::vector<Timestamp> initialTimestamps = {
          system_clock::from_time_t(0),
          system_clock::from_time_t(10),
          system_clock::from_time_t(20)
      };
      IoTData<double> data(initialData, initialTimestamps);

      std::vector<Timestamp> newTs = {
          system_clock::from_time_t(4),  // Closer to 0 -> 0.0
          system_clock::from_time_t(6),  // Closer to 10 -> 10.0
          system_clock::from_time_t(10), // Exact point -> 10.0
          system_clock::from_time_t(16)  // Closer to 20 -> 20.0
      };

      std::vector<double> expected = {0.0, 10.0, 10.0, 20.0};
      std::vector<double> result = data.interpolateData(newTs, InterpolationMethod::NEAREST_NEIGHBOR);

      ASSERT_EQ(result.size(), expected.size());
      for(size_t i=0; i<result.size(); ++i) {
          EXPECT_DOUBLE_EQ(result[i], expected[i]);
      }
}

TEST_F(IoTDataTest, InterpolateUnsortedNewTimestamps) {
     IoTData<double> data({0.0, 10.0}, {system_clock::from_time_t(0), system_clock::from_time_t(10)});
     std::vector<Timestamp> newTsUnsorted = {
         system_clock::from_time_t(8),
         system_clock::from_time_t(2) // Out of order
     };
     EXPECT_THROW(data.interpolateData(newTsUnsorted), IoTDataException);
}


// --- Main function for running tests ---
// (Handled by linking GTest::gtest_main)
