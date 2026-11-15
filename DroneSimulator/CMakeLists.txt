add_executable(DroneSimulator
    src/main.cpp
    src/simulator.cpp
    src/drone.cpp
    src/world.cpp
    src/udp_socket.cpp
)

target_include_directories(DroneSimulator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(DroneSimulator PRIVATE
    drone_protocol
)
