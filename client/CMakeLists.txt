# =============================================================================
#  client - game client executable (SFML + Dear ImGui via imgui-sfml)
# =============================================================================

add_executable(rts_client
    src/main.cpp
)

target_include_directories(rts_client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(rts_client
    PRIVATE
        rts::common
        rts_project_options
        rts_project_warnings

        # Rendering / windowing
        SFML::Graphics
        SFML::Window
        SFML::System
        SFML::Network

        # Debug UI (imgui-sfml pulls in imgui transitively, but we link
        # both explicitly for clarity)
        ImGui-SFML::ImGui-SFML
        imgui::imgui

        # Networking (client side - we only need ENet here)
        enet::enet
)

set_target_properties(rts_client PROPERTIES
    OUTPUT_NAME "rts-client"
    WIN32_EXECUTABLE FALSE   # keep console open during development
)

if(WIN32)
    target_link_libraries(rts_client PRIVATE ws2_32 winmm)
endif()