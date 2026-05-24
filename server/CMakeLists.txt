# =============================================================================
#  server - authoritative game server executable
# =============================================================================

add_executable(rts_server
    src/main.cpp
)

target_include_directories(rts_server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(rts_server
    PRIVATE
        rts::common
        rts_project_options   # Propaguje opcje globalne, sanitizery oraz warningi

        # Networking
        Boost::system         # TCP / sesje i autoryzacja 
        enet::enet            # UDP / ticki symulacji 

        # Database
        libpqxx::pqxx         # Asynchroniczny PostgreSQL 

        # Observability
        prometheus-cpp::core  # Metryki serwera dla Prometheusa
        prometheus-cpp::pull
)

# Useful runtime artifact name regardless of platform
set_target_properties(rts_server PROPERTIES
    OUTPUT_NAME "rts-server"
)

# On Windows, link the Winsock library required by Boost.Asio and ENet
if(WIN32)
    target_link_libraries(rts_server PRIVATE ws2_32 winmm)
endif()