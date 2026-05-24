# =============================================================================
#  common - shared library used by both server and client
#  (protocol definitions, math, utility types, ECS components, ...)
# =============================================================================

add_library(rts_common STATIC
    # Source files will be added here as the project grows.
    # Placeholder so CMake accepts an empty target:
    src/placeholder.cpp
)

# Alias so other targets can link via rts::common
add_library(rts::common ALIAS rts_common)

target_include_directories(rts_common
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(rts_common
    PUBLIC
        rts_project_options
        rts_project_warnings
        EnTT::EnTT
        flatbuffers::flatbuffers
        spdlog::spdlog
)

set_target_properties(rts_common PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
