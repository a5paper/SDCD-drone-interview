add_executable(DroneSimulator
    src/main.cpp
    src/simulator.cpp
    src/drone.cpp
    src/world.cpp
    ${CMAKE_SOURCE_DIR}/generated/cpp/commands.pb.cc
)

target_include_directories(DroneSimulator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/generated/cpp
)

target_link_libraries(DroneSimulator PRIVATE
    protobuf::libprotobuf
)