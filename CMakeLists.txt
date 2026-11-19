cmake_minimum_required(VERSION 3.21)

project(DroneProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# apt ships no protobuf config package, so fall back to the module.
find_package(Protobuf CONFIG QUIET)
if(NOT Protobuf_FOUND)
  find_package(Protobuf REQUIRED)
endif()
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs highgui)
find_package(GTest REQUIRED)

enable_testing()

add_subdirectory(protocols)
add_subdirectory(DroneSimulator)
