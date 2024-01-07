#!/bin/bash

# Define filenames
DATA_FILE="sensor_data.txt"
EXPORTED_FILE="exported_data.txt"

# Compile and run the main example
g++ -std=c++11 -I./include -o IoTDataExample ./examples/main.cpp ./src/IoTData.cpp ./src/IoTDataException.cpp

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful. Running IoTDataExample..."
    ./IoTDataExample $DATA_FILE $EXPORTED_FILE

    # Check if the example executed successfully
    if [ $? -eq 0 ]; then
        echo "Example executed successfully."

        # Additional processing using the IoT data library
        echo "Performing additional processing using the IoT data library..."
        # Use if applicable, can be removed.

        # Clean up
        rm IoTDataExample $EXPORTED_FILE
    else
        echo "Error: Failed to execute IoTDataExample."
    fi
else
    echo "Error: Compilation failed."
fi
