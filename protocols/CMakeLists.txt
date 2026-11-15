add_library(drone_protocol STATIC)

protobuf_generate(
    TARGET drone_protocol
    LANGUAGE cpp
    PROTOS ${CMAKE_CURRENT_SOURCE_DIR}/commands.proto
    IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
    PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)

target_include_directories(drone_protocol PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(drone_protocol PUBLIC
    protobuf::libprotobuf
)
