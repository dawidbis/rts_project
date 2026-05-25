# =============================================================================
#  tests - unit tests (Catch2)
# =============================================================================
include(Catch)

# ----- Helper: register a test executable in one place -----
function(rts_add_test name)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs SOURCES LINKS)
    cmake_parse_arguments(T "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_executable(${name} ${T_SOURCES})
    target_link_libraries(${name}
        PRIVATE
            rts::common
            Catch2::Catch2WithMain
            rts_project_warnings
            ${T_LINKS}
    )
    catch_discover_tests(${name})
endfunction()

add_subdirectory(common)

if(RTS_BUILD_SERVER)
    add_subdirectory(server)
endif()

if(RTS_BUILD_CLIENT)
    add_subdirectory(client)
endif()