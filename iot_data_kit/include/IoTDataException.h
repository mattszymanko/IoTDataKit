// IoTDataException.h
#ifndef IOT_DATA_EXCEPTION_H
#define IOT_DATA_EXCEPTION_H

#include <stdexcept>
#include <string>

class IoTDataException : public std::runtime_error {
public:
    explicit IoTDataException(const std::string& message) : std::runtime_error(message) {}
};

class IoTDataEmptyException : public IoTDataException {
public:
    explicit IoTDataEmptyException(const std::string& message) : IoTDataException(message) {}
};

class IoTDataInsufficientException : public IoTDataException {
public:
    explicit IoTDataInsufficientException(const std::string& message) : IoTDataException(message) {}
};

class IoTDataFileException : public IoTDataException {
public:
    explicit IoTDataFileException(const std::string& message) : IoTDataException(message) {}
};

#endif // IOT_DATA_EXCEPTION_H
