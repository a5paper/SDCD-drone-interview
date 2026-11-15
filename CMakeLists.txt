cmake_minimum_required(VERSION 3.16)

project(DroneProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Protobuf REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs highgui)

add_subdirectory(protocols)
add_subdirectory(DroneSimulator)
