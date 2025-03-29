// include/IoTDataException.h
#ifndef IOT_DATA_EXCEPTION_H
#define IOT_DATA_EXCEPTION_H

#include <stdexcept>
#include <string>

/**
 * @brief Base class for exceptions thrown by the IoTData library.
 */
class IoTDataException : public std::runtime_error {
public:
    /**
     * @brief Constructs an IoTDataException.
     * @param message The error message.
     */
    explicit IoTDataException(const std::string& message) : std::runtime_error(message) {}
};

/**
 * @brief Exception thrown when an operation requires data but the dataset is empty.
 */
class IoTDataEmptyException : public IoTDataException {
public:
    explicit IoTDataEmptyException(const std::string& message) : IoTDataException(message) {}
};

/**
 * @brief Exception thrown when an operation requires a minimum amount of data points,
 *        but the dataset has fewer than required.
 */
class IoTDataInsufficientException : public IoTDataException {
public:
    explicit IoTDataInsufficientException(const std::string& message) : IoTDataException(message) {}
};

/**
 * @brief Exception thrown during file operations (import/export) errors.
 */
class IoTDataFileException : public IoTDataException {
public:
    explicit IoTDataFileException(const std::string& message) : IoTDataException(message) {}
};

#endif // IOT_DATA_EXCEPTION_H
