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

    return std::accumulate(data.begin(), data.end(), 0.0) / data.size();
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

    inputFile.close();
}

void IoTData::plotData() const {
    // Hypothetical data visualization code (not implemented)
    std::cout << "Data plot: [";
    for (const double& value : data) {
        std::cout << value << ", ";
    }
    std::cout << "]" << std::endl;
}
