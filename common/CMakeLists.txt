# =============================================================================
#  RTS Engine - Common Module
# =============================================================================

# 1. Definiujemy œcie¿ki do schematu FlatBuffers
set(FBS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/fbs/rts_protocol.fbs")
set(FBS_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(FBS_GEN_HDR "${FBS_OUT_DIR}/rts_protocol_generated.h")

# Upewniamy siê, ¿e folder na wygenerowane nag³ówki istnieje
file(MAKE_DIRECTORY "${FBS_OUT_DIR}")

# 2. Generowanie nag³ówka przez cel z Conana!
add_custom_command(
    OUTPUT "${FBS_GEN_HDR}"
    COMMAND "$<TARGET_FILE:flatbuffers::flatc>" -c -o "${FBS_OUT_DIR}" "${FBS_SRC}"
    DEPENDS "${FBS_SRC}"
    COMMENT "Kompilowanie schematu FlatBuffers: rts_protocol.fbs"
    VERBATIM
)

# 3. Definicja biblioteki `rts_common`
add_library(rts_common
    src/network/Serializer.cpp
    "${FBS_GEN_HDR}"
)

# Alias, aby g³ówny projekt móg³ linkowaæ rts::common
add_library(rts::common ALIAS rts_common)

# 4. Udostêpnianie œcie¿ek nag³ówków
target_include_directories(rts_common PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${FBS_OUT_DIR}>
)

# 5. Pod³¹czenie zale¿noœci
target_link_libraries(rts_common PUBLIC
    rts_project_options
    flatbuffers::flatbuffers
    enet::enet              
    spdlog::spdlog
)