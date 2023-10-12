cmake_minimum_required(VERSION 3.10)

project(LoRaImageTransfer C)

# Add include directories
include_directories(include)

# Create a list of source files in the src directory
file(GLOB SOURCES src/*.c)

# Create an executable
add_executable(${PROJECT_NAME} ${SOURCES})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

# Link libraries
target_link_libraries(${PROJECT_NAME} pigpio m)