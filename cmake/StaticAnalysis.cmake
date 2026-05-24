# StaticAnalysis.cmake
# Enable clang-tidy as a per-target compile-time linter.
# Configuration lives in the top-level .clang-tidy file.

function(rts_enable_clang_tidy)
    find_program(CLANGTIDY clang-tidy)
    if(NOT CLANGTIDY)
        message(WARNING "clang-tidy requested but executable not found")
        return()
    endif()

    set(CMAKE_CXX_CLANG_TIDY
        ${CLANGTIDY}
        -extra-arg=-Wno-unknown-warning-option
        CACHE STRING "clang-tidy command line" FORCE
    )
    message(STATUS "clang-tidy enabled: ${CLANGTIDY}")
endfunction()
