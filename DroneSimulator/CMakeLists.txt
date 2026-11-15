add_executable(DroneSimulator
    src/main.cpp
)

target_include_directories(DroneSimulator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
