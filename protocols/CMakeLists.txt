add_library(drone_protocol STATIC)

set(PROTO_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/commands.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/gps.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/telemetry.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/video.proto
)

# C++ for the simulator, Python (*_pb2.py) for the GCS.
foreach(LANG cpp python)
    protobuf_generate(
        TARGET drone_protocol
        LANGUAGE ${LANG}
        PROTOS ${PROTO_FILES}
        IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
    )
endforeach()

target_include_directories(drone_protocol SYSTEM PUBLIC
    ${PROJECT_BINARY_DIR}
)

target_link_libraries(drone_protocol PUBLIC
    protobuf::libprotobuf
)
