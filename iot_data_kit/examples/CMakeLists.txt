# examples/CMakeLists.txt

# Find the library (defined in parent scope)
if(NOT TARGET iot_data_kit)
    message(FATAL_ERROR "iot_data_kit target not found. Build examples from the top-level directory.")
endif()

# Example executable
add_executable(example_main main.cpp)

# Link the library to the example executable
# For INTERFACE libraries, target_link_libraries handles include directories etc.
target_link_libraries(example_main PRIVATE iot_data_kit)

# Optional: Installation for the example
# install(TARGETS example_main DESTINATION bin)
