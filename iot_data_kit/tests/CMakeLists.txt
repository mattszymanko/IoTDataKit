# tests/CMakeLists.txt

# --- GoogleTest ---
# Find the GoogleTest dependency, fetched in the parent scope
# If using CMake < 3.11, find_package(GTest REQUIRED) might be needed after manual download/install
if(NOT TARGET GTest::gtest_main)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip # Or use GIT_REPOSITORY/GIT_TAG
        # Consider adding URL_HASH SHA256=... for integrity
    )
    # Set options to avoid building unnecessary parts of googletest
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE) # If needed on Windows
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)
    message(STATUS "Fetched and configured GoogleTest.")
endif()


# --- Test Executable ---
add_executable(iot_data_kit_tests IoTDataTests.cpp)

# --- Link Dependencies ---
# Link the main library interface target
# Link GoogleTest's main target (includes gtest and runs tests)
target_link_libraries(iot_data_kit_tests PRIVATE
    iot_data_kit # Our library interface
    GTest::gtest_main # GoogleTest framework + main function
)

# --- Add Test to CTest ---
# This allows running tests with 'ctest' command from the build directory
include(GoogleTest)
gtest_add_tests(TARGET iot_data_kit_tests AUTO)

# Optional: Installation for the test executable (usually not needed)
# install(TARGETS iot_data_kit_tests DESTINATION bin)
