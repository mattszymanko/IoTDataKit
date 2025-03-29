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

# --- Dependencies ---
include(FetchContent)

# nlohmann_json (for DataSet JSON I/O)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3 # Use a specific stable tag
)
FetchContent_MakeAvailable(nlohmann_json)
message(STATUS "Fetched and configured nlohmann_json.")


# --- Library Definition (Now Header-Only Interface Library) ---
# Since classes are templated, implementation goes in the headers.
# We create an INTERFACE library to manage include directories and dependencies.
add_library(iot_data_kit INTERFACE)
target_include_directories(iot_data_kit INTERFACE include)

# Add dependency requirement to our interface library
# Targets linking iot_data_kit will also get include dirs for nlohmann_json
target_link_libraries(iot_data_kit INTERFACE nlohmann_json::nlohmann_json)


# --- Build Examples ---
if(IOTDATAKIT_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# --- Build Tests ---
if(IOTDATAKIT_BUILD_TESTS)
    enable_testing() # Enable CTest support

    # GoogleTest (dependency for tests)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        # Consider adding URL_HASH SHA256=... for integrity
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE) # If needed on Windows
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    message(STATUS "Fetched and configured GoogleTest.")

    add_subdirectory(tests)
endif()

# --- Installation (Optional, Example - Needs refinement for Interface Libs) ---
# include(GNUInstallDirs)
# # Need to install headers explicitly
# install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/iot_data_kit)
# # And potentially create a CMake config file for find_package
# include(CMakePackageConfigHelpers)
# configure_package_config_file(...)
# install(EXPORT ...)

# --- Project Summary ---
message(STATUS "--------------------------------------------------")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Examples: ${IOTDATAKIT_BUILD_EXAMPLES}")
message(STATUS "Build Tests: ${IOTDATAKIT_BUILD_TESTS}")
message(STATUS "Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Using nlohmann_json: YES")
message(STATUS "Using GoogleTest: ${IOTDATAKIT_BUILD_TESTS}")
message(STATUS "--------------------------------------------------")
