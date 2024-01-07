// IoTDataException.cpp
#include "IoTDataException.h"

IoTDataException::IoTDataException(const std::string& message) : std::runtime_error(message) {}

IoTDataEmptyException::IoTDataEmptyException(const std::string& message) : IoTDataException(message) {}

IoTDataInsufficientException::IoTDataInsufficientException(const std::string& message)
    : IoTDataException(message) {}

IoTDataFileException::IoTDataFileException(const std::string& message) : IoTDataException(message) {}
