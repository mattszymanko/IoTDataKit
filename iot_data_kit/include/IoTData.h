// IoTData.h
#ifndef IOT_DATA_H
#define IOT_DATA_H

#include <vector>
#include <functional>

enum class InterpolationMethod {
    LINEAR,
    NEAREST_NEIGHBOR,
    CUBIC_SPLINE
};

class IoTData {
private:
    std::vector<double> data;
    std::vector<double> timestamps;  // New member to store timestamps

    // Helper function for cubic spline interpolation
    std::vector<double> calculateSplineCoefficients(const std::vector<double>& x, const std::vector<double>& y) const;

public:
    // Constructor
    IoTData(const std::vector<double>& initialData);
    IoTData(const std::vector<double>& initialData, const std::vector<double>& initialTimestamps);

    // Basic data manipulation functions
    void appendData(double newData, double timestamp);
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

    // Data resampling functions
    std::vector<double> resampleData(size_t targetSize) const;

    // New Data Interpolation function
    std::vector<double> interpolateData(const std::vector<double>& newTimestamps, 
                                        InterpolationMethod method = InterpolationMethod::LINEAR) const;
};

#endif // IOT_DATA_H
