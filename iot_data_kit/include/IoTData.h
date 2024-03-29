// IoTData.h
#ifndef IOT_DATA_H
#define IOT_DATA_H

#include <vector>

class IoTData {
private:
    std::vector<double> data;

public:
    // Constructor
    IoTData(const std::vector<double>& initialData);

    // Basic data manipulation functions
    void appendData(double newData);
    void clearData();
    size_t getDataSize() const;

    // Data filtering functions
    void filterOutliers(double threshold);

    // Statistical analysis functions
    double calculateMean() const;
    double calculateStandardDeviation() const;

    // Data transformation functions
    void scaleData(double scaleFactor);
    void normalizeData();

    // Data export/import functions
    void exportDataToFile(const std::string& filename) const;
    void importDataFromFile(const std::string& filename);

    // Data visualization functions
    void plotData() const;

    // Data trimming functions 
    void trimData(double trimPercentage);

    // Moving average calculation functions
    std::vector<double> calculateMovingAverage(size_t windowSize) const;

    // Rolling mean calculation functions
    std::vector<double> calculateRollingMean(size_t windowSize) const;

    // Data resampling fuctions
    std::vector<double> resampleData(size_t targetSize) const;
};

#endif // IOT_DATA_H
