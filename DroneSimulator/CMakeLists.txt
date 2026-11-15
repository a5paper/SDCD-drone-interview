add_compile_options(-Wall -Wextra -Wpedantic $<$<CONFIG:Debug>:-g3>)

# core: config constants, INI reader, network config loader
add_library(drone_core STATIC
    src/core/ini_file.cpp
    src/core/network_config.cpp
)
target_include_directories(drone_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# sim: drone, world and simulation step
add_library(drone_sim STATIC
    src/sim/drone.cpp
    src/sim/world.cpp
    src/sim/simulator.cpp
)
target_link_libraries(drone_sim PUBLIC drone_core drone_protocol)

# render: top-down frame and JPEG encoding
add_library(drone_render STATIC
    src/render/renderer.cpp
    src/render/jpeg_renderer.cpp
)
target_link_libraries(drone_render
    PUBLIC drone_sim opencv_core
    PRIVATE opencv_imgproc opencv_imgcodecs
)

# network: UDP socket, command receiving, video streaming
add_library(drone_network STATIC
    src/network/udp_socket.cpp
    src/network/command_receiver.cpp
    src/network/video_streamer.cpp
)
target_link_libraries(drone_network PUBLIC drone_core drone_protocol)

add_executable(DroneSimulator src/main.cpp)
target_compile_definitions(DroneSimulator PRIVATE
    DRONE_SIMULATOR_DEFAULT_CONFIG="${PROJECT_SOURCE_DIR}/config/network.ini"
)
target_link_libraries(DroneSimulator PRIVATE
    drone_sim
    drone_render
    drone_network
    opencv_highgui
    opencv_imgcodecs
)
