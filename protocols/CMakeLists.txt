add_library(drone_protocol STATIC)

protobuf_generate(
    TARGET drone_protocol
    LANGUAGE cpp
    PROTOS
        ${CMAKE_CURRENT_SOURCE_DIR}/commands.proto
        ${CMAKE_CURRENT_SOURCE_DIR}/telemetry.proto
        ${CMAKE_CURRENT_SOURCE_DIR}/video.proto
    IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
    PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)

target_include_directories(drone_protocol SYSTEM PUBLIC
    ${PROJECT_BINARY_DIR}
)

target_link_libraries(drone_protocol PUBLIC
    protobuf::libprotobuf
)
