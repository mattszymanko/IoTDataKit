# CMakeLists.txt (Top Level)

cmake_minimum_required(VERSION 3.14) # Increased for FetchContent best practices
project(iot_data_kit VERSION 0.9.0 LANGUAGES CXX)

# --- Options ---
option(IOTDATAKIT_BUILD_EXAMPLES "Build examples for IoTDataKit" ON)
option(IOTDATAKIT_BUILD_TESTS "Build tests for IoTDataKit" ON)

# --- C++ Standard ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Prefer standard features

# --- Library Definition (Now Header-Only Interface Library) ---
# Since IoTData is templated, implementation goes in the header.
# We create an INTERFACE library to manage include directories and dependencies.
add_library(iot_data_kit INTERFACE)
target_include_directories(iot_data_kit INTERFACE include)

# --- Build Examples ---
if(IOTDATAKIT_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# --- Build Tests ---
if(IOTDATAKIT_BUILD_TESTS)
    enable_testing() # Enable CTest support
    add_subdirectory(tests)
endif()

# --- Installation (Optional, Example) ---
# include(GNUInstallDirs)
# install(TARGETS iot_data_kit # This doesn't work directly for INTERFACE libraries
#         EXPORT IoTDataKitTargets
#         LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#         ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
# install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/iot_data_kit) # Install headers
# install(EXPORT IoTDataKitTargets
#         FILE IoTDataKitTargets.cmake
#         NAMESPACE IoTDataKit::
#         DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/IoTDataKit)

# --- Project Summary ---
message(STATUS "--------------------------------------------------")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Examples: ${IOTDATAKIT_BUILD_EXAMPLES}")
message(STATUS "Build Tests: ${IOTDATAKIT_BUILD_TESTS}")
message(STATUS "Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "--------------------------------------------------")
