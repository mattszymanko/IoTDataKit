# CMakeLists.txt

cmake_minimum_required(VERSION 3.12)
project(iot_data_kit)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the source files
set(SOURCES
    src/IoTData.cpp
    src/IoTDataException.cpp
)

# Set the header files
set(HEADERS
    include/IoTData.h
    include/IoTDataException.h
)

# Create a library target
add_library(iot_data_kit ${SOURCES} ${HEADERS})

# Specify include directories for the library
target_include_directories(iot_data_kit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Example executable
add_executable(example_main examples/main.cpp)

# Link the library to the example executable
target_link_libraries(example_main iot_data_kit)
