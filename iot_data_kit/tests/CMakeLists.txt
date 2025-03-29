# tests/CMakeLists.txt

# Find dependencies (already fetched/available from parent scope)
if(NOT TARGET GTest::gtest_main)
    message(FATAL_ERROR "GoogleTest target not found. Build tests from the top-level directory.")
endif()
if(NOT TARGET nlohmann_json::nlohmann_json)
     message(FATAL_ERROR "nlohmann_json target not found. Build tests from the top-level directory.")
endif()
if(NOT TARGET iot_data_kit)
    message(FATAL_ERROR "iot_data_kit target not found. Build tests from the top-level directory.")
endif()


# --- Test Executable ---
add_executable(iot_data_kit_tests
    IoTDataTests.cpp
    DataSetTests.cpp # Add the new test file
)

# --- Link Dependencies ---
# Link the main library interface target
# Link GoogleTest's main target (includes gtest and runs tests)
# nlohmann_json is linked implicitly via iot_data_kit interface library
target_link_libraries(iot_data_kit_tests PRIVATE
    iot_data_kit       # Our library interface (pulls in nlohmann_json)
    GTest::gtest_main  # GoogleTest framework + main function
)

# --- Add Test to CTest ---
# This allows running tests with 'ctest' command from the build directory
include(GoogleTest)
gtest_add_tests(TARGET iot_data_kit_tests AUTO)

# Optional: Installation for the test executable (usually not needed)
# install(TARGETS iot_data_kit_tests DESTINATION bin)
