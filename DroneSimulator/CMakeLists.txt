add_executable(DroneSimulator
    src/main.cpp
    src/simulator.cpp
    src/drone.cpp
    src/world.cpp
)

target_include_directories(DroneSimulator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
