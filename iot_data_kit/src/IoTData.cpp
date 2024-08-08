// IoTData.cpp
#include "IoTData.h"
#include "IoTDataException.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <cmath>

IoTData::IoTData(const std::vector<double>& initialData) : data(initialData) {
    timestamps.resize(initialData.size());
    std::iota(timestamps.begin(), timestamps.end(), 0.0);
}

IoTData::IoTData(const std::vector<double>& initialData, const std::vector<double>& initialTimestamps) 
    : data(initialData), timestamps(initialTimestamps) {
    if (data.size() != timestamps.size()) {
        throw IoTDataException("Error: Number of data points and timestamps must match.");
    }
}

void IoTData::appendData(double newData, double timestamp) {
    data.push_back(newData);
    timestamps.push_back(timestamp);
}

void IoTData::clearData() {
    data.clear();
    timestamps.clear();
}

size_t IoTData::getDataSize() const {
    return data.size();
}

void IoTData::filterOutliers(double threshold) {
    auto it = std::remove_if(data.begin(), data.end(),
                             [threshold](double value) { return std::abs(value) > threshold; });
    timestamps.erase(timestamps.begin() + std::distance(data.begin(), it), timestamps.end());
    data.erase(it, data.end());
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

    for (size_t i = 0; i < data.size(); ++i) {
        outputFile << timestamps[i] << "," << data[i] << '\n';
    }

    outputFile.close();
}

void IoTData::importDataFromFile(const std::string& filename) {
    std::ifstream inputFile(filename);

    if (!inputFile.is_open()) {
        throw IoTDataFileException("Error: Unable to open the file for data import.");
    }

    data.clear();
    timestamps.clear();

    double timestamp, value;
    char comma;
    while (inputFile >> timestamp >> comma >> value) {
        if (comma != ',') {
            throw IoTDataFileException("Error: Invalid file format. Expected comma-separated values.");
        }
        timestamps.push_back(timestamp);
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
    for (size_t i = 0; i < data.size(); ++i) {
        std::cout << "(" << timestamps[i] << ", " << data[i] << "), ";
    }
    std::cout << "]" << std::endl;
}

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

void IoTData::trimData(double trimPercentage) {
    if (data.empty()) {
        throw IoTDataEmptyException("Error: No data available for trimming.");
    }

    size_t trimCount = static_cast<size_t>(data.size() * trimPercentage / 200.0);

    if (trimCount > 0) {
        data.erase(data.begin(), data.begin() + trimCount);
        data.erase(data.end() - trimCount, data.end());
        timestamps.erase(timestamps.begin(), timestamps.begin() + trimCount);
        timestamps.erase(timestamps.end() - trimCount, timestamps.end());
    }
}

std::vector<double> IoTData::calculateMovingAverage(size_t windowSize) const {
    if (data.empty()) {
        throw IoTDataEmptyException("Error: No data available for moving average calculation.");
    }

    std::vector<double> movingAverage;
    movingAverage.reserve(data.size());

    double sum = 0.0;

    for (size_t i = 0; i < data.size(); ++i) {
        sum += data[i];
        if (i >= windowSize) {
            sum -= data[i - windowSize];
            movingAverage.push_back(sum / windowSize);
        } else {
            movingAverage.push_back(sum / (i + 1));
        }
    }

    return movingAverage;
}

std::vector<double> IoTData::interpolateData(const std::vector<double>& newTimestamps, InterpolationMethod method) const {
    if (data.empty() || timestamps.empty()) {
        throw IoTDataEmptyException("Error: No data available for interpolation.");
    }

    std::vector<double> interpolatedData;
    interpolatedData.reserve(newTimestamps.size());

    switch (method) {
        case InterpolationMethod::LINEAR:
            for (double t : newTimestamps) {
                auto it = std::lower_bound(timestamps.begin(), timestamps.end(), t);
                if (it == timestamps.begin()) {
                    interpolatedData.push_back(data.front());
                } else if (it == timestamps.end()) {
                    interpolatedData.push_back(data.back());
                } else {
                    size_t index = std::distance(timestamps.begin(), it);
                    double t0 = timestamps[index - 1];
                    double t1 = timestamps[index];
                    double y0 = data[index - 1];
                    double y1 = data[index];
                    double y = y0 + (y1 - y0) * (t - t0) / (t1 - t0);
                    interpolatedData.push_back(y);
                }
            }
            break;

        case InterpolationMethod::NEAREST_NEIGHBOR:
            for (double t : newTimestamps) {
                auto it = std::lower_bound(timestamps.begin(), timestamps.end(), t);
                if (it == timestamps.begin()) {
                    interpolatedData.push_back(data.front());
                } else if (it == timestamps.end()) {
                    interpolatedData.push_back(data.back());
                } else {
                    size_t index = std::distance(timestamps.begin(), it);
                    double prev_diff = std::abs(t - timestamps[index - 1]);
                    double next_diff = std::abs(t - timestamps[index]);
                    interpolatedData.push_back(prev_diff < next_diff ? data[index - 1] : data[index]);
                }
            }
            break;

        case InterpolationMethod::CUBIC_SPLINE:
            {
                std::vector<double> coeffs = calculateSplineCoefficients(timestamps, data);
                for (double t : newTimestamps) {
                    auto it = std::lower_bound(timestamps.begin(), timestamps.end(), t);
                    if (it == timestamps.begin()) {
                        interpolatedData.push_back(data.front());
                    } else if (it == timestamps.end()) {
                        interpolatedData.push_back(data.back());
                    } else {
                        size_t index = std::distance(timestamps.begin(), it);
                        double h = timestamps[index] - timestamps[index - 1];
                        double a = (timestamps[index] - t) / h;
                        double b = (t - timestamps[index - 1]) / h;
                        double y = a * data[index - 1] + b * data[index] +
                                   ((a * a * a - a) * coeffs[index - 1] + (b * b * b - b) * coeffs[index]) * (h * h) / 6.0;
                        interpolatedData.push_back(y);
                    }
                }
            }
            break;
    }

    return interpolatedData;
}

std::vector<double> IoTData::calculateSplineCoefficients(const std::vector<double>& x, const std::vector<double>& y) const {
    size_t n = x.size();
    std::vector<double> a(n), b(n), c(n), d(n);
    std::vector<double> h(n - 1);

    for (size_t i = 0; i < n - 1; ++i) {
        h[i] = x[i + 1] - x[i];
        b[i] = (y[i + 1] - y[i]) / h[i];
    }

    a[0] = 0;
    c[0] = 0;
    d[0] = 0;

    for (size_t i = 1; i < n - 1; ++i) {
        a[i] = h[i - 1];
        c[i] = 2 * (h[i - 1] + h[i]);
        d[i] = h[i];
        b[i] = 6 * (b[i] - b[i - 1]);
    }

    c[n - 1] = 0;
    b[n - 1] = 0;

    for (size_t i = 1; i < n; ++i) {
        double m = a[i] / c[i - 1];
        c[i] -= m * d[i - 1];
        b[i] -= m * b[i - 1];
    }

    std::vector<double> coeffs(n);
    coeffs[n - 1] = b[n - 1] / c[n - 1];

    for (int i = n - 2; i >= 0; --i) {
        coeffs[i] = (b[i] - d[i] * coeffs[i + 1]) / c[i];
    }

    return coeffs;
}
