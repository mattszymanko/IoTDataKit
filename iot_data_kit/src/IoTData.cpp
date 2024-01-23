// IoTData.cpp
#include "IoTData.h"
#include "IoTDataException.h"
#include <iostream>
#include <fstream>

IoTData::IoTData(const std::vector<double>& initialData) : data(initialData) {}

void IoTData::appendData(double newData) {
    data.push_back(newData);
}

void IoTData::clearData() {
    data.clear();
}

size_t IoTData::getDataSize() const {
    return data.size();
}

void IoTData::filterOutliers(double threshold) {
    data.erase(std::remove_if(data.begin(), data.end(),
                              [threshold](double value) { return std::abs(value) > threshold; }),
               data.end());
}

double IoTData::calculateMean() const {
    if (data.empty()) {
        throw IoTDataEmptyException("Error: No data available for mean calculation.");
    }

    if (std::any_of(data.begin(), data.end(), std::isnan)) {
        throw IoTDataException("Error: Data contains NaN (Not a Number) values.");
    }

    if (std::any_of(data.begin(), data.end(), std::isinf)) {
        throw IoTDataException("Error: Data contains infinite values.");
    }

    if (std::any_of(data.begin(), data.end(), [](double value) { return std::isnan(value) || std::isinf(value); })) {
        throw IoTDataException("Error: Data contains invalid values (NaN or infinity).");
    }

    double sum = std::accumulate(data.begin(), data.end(), 0.0);

    if (std::isnan(sum) || std::isinf(sum)) {
        throw IoTDataException("Error: Sum of data values resulted in an invalid value (NaN or infinity).");
    }

    return sum / data.size();
}

double IoTData::calculateStandardDeviation() const {
    if (data.size() < 2) {
        throw IoTDataInsufficientException("Error: Insufficient data for standard deviation calculation.");
    }

    double mean = calculateMean();
    double sum = 0.0;

    for (const double& value : data) {
        sum += std::pow(value - mean, 2);
    }

    return std::sqrt(sum / data.size());
}

void IoTData::scaleData(double scaleFactor) {
    std::transform(data.begin(), data.end(), data.begin(),
                   [scaleFactor](double value) { return value * scaleFactor; });
}

void IoTData::normalizeData() {
    double mean = calculateMean();
    double stdev = calculateStandardDeviation();

    std::transform(data.begin(), data.end(), data.begin(),
                   [mean, stdev](double value) { return (value - mean) / stdev; });
}

void IoTData::exportDataToFile(const std::string& filename) const {
    std::ofstream outputFile(filename);

    if (!outputFile.is_open()) {
        throw IoTDataFileException("Error: Unable to open the file for data export.");
    }

    for (const double& value : data) {
        outputFile << value << '\n';
    }

    outputFile.close();
}

void IoTData::importDataFromFile(const std::string& filename) {
    std::ifstream inputFile(filename);

    if (!inputFile.is_open()) {
        throw IoTDataFileException("Error: Unable to open the file for data import.");
    }

    double value;
    while (inputFile >> value) {
        data.push_back(value);
    }

    if (data.empty()) {
        throw IoTDataFileException("Error: No data found in the input file.");
    }

    inputFile.close();
}

void IoTData::plotData() const {
    // Hypothetical data visualization code (not implemented at this time)
    std::cout << "Data plot: [";
    for (const double& value : data) {
        std::cout << value << ", ";
    }
    std::cout << "]" << std::endl;
}

// Rolling Mean Calculation
std::vector<double> IoTData::calculateRollingMean(size_t windowSize) const {
    if (data.empty()) {
        throw IoTDataEmptyException("Error: No data available for rolling mean calculation.");
    }

    std::vector<double> rollingMean;
    rollingMean.reserve(data.size());

    double sum = 0.0;

    for (size_t i = 0; i < data.size(); ++i) {
        sum += data[i];
        if (i >= windowSize) {
            sum -= data[i - windowSize];
            rollingMean.push_back(sum / windowSize);
        } else {
            rollingMean.push_back(sum / (i + 1));
        }
    }

    return rollingMean;
}

// Data Resampling
std::vector<double> IoTData::resampleData(size_t targetSize) const {
    if (data.empty()) {
        throw IoTDataEmptyException("Error: No data available for resampling.");
    }

    std::vector<double> resampledData;
    resampledData.reserve(targetSize);

    double step = static_cast<double>(data.size() - 1) / (targetSize - 1);

    for (size_t i = 0; i < targetSize; ++i) {
        double index = i * step;
        size_t lowerIndex = static_cast<size_t>(std::floor(index));
        size_t upperIndex = static_cast<size_t>(std::ceil(index));

        double lowerValue = data[lowerIndex];
        double upperValue = data[upperIndex];

        double interpolatedValue = lowerValue + (index - lowerIndex) * (upperValue - lowerValue);
        resampledData.push_back(interpolatedValue);
    }

    return resampledData;
}

// Data Trimming
void IoTData::trimData(double trimPercentage) {
    if (data.empty()) {
        throw IoTDataEmptyException("Error: No data available for trimming.");
    }

    size_t trimCount = static_cast<size_t>(data.size() * trimPercentage / 200.0);

    if (trimCount > 0) {
        data.erase(data.begin(), data.begin() + trimCount);
        data.erase(data.end() - trimCount, data.end());
    }
}
