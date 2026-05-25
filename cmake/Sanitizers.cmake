# Sanitizers.cmake
# Wire ASan / UBSan / TSan into an INTERFACE target.
# Sanitizers require both compile-time and link-time flags.

function(rts_enable_sanitizers target)
    if(NOT (RTS_ENABLE_ASAN OR RTS_ENABLE_UBSAN OR RTS_ENABLE_TSAN))
        return()
    endif()

    if(RTS_ENABLE_ASAN AND RTS_ENABLE_TSAN)
        message(FATAL_ERROR "AddressSanitizer and ThreadSanitizer are mutually exclusive")
    endif()

    set(sanitizers "")
    if(RTS_ENABLE_ASAN)
        list(APPEND sanitizers "address")
    endif()
    if(RTS_ENABLE_UBSAN)
        list(APPEND sanitizers "undefined")
    endif()
    if(RTS_ENABLE_TSAN)
        list(APPEND sanitizers "thread")
    endif()

    list(JOIN sanitizers "," sanitizer_flags)

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} INTERFACE
            -fsanitize=${sanitizer_flags}
            -fno-omit-frame-pointer
            -g
        )
        target_link_options(${target} INTERFACE
            -fsanitize=${sanitizer_flags}
        )
    elseif(MSVC AND RTS_ENABLE_ASAN)
        # MSVC supports /fsanitize=address since VS 2019 16.9
        target_compile_options(${target} INTERFACE /fsanitize=address)
        # Note: UBSan/TSan are NOT supported on MSVC
    else()
        message(WARNING "Sanitizers requested but compiler does not support them; skipping")
    endif()

    message(STATUS "Sanitizers enabled: ${sanitizer_flags}")
endfunction()
