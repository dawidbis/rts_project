# CompilerWarnings.cmake
# Apply a strict, modern warning set across MSVC, GCC and Clang.

function(rts_set_project_warnings target)
    set(MSVC_WARNINGS
        /W4         # reasonable, sensible warning level
        /w14242     # conversion possible loss of data
        /w14254     # similar
        /w14263     # member fn does not override base
        /w14265     # class has virtual fns but no virtual dtor
        /w14287     # unsigned/negative constant mismatch
        /we4289     # for-loop scope (treat as error)
        /w14296     # expression is always true/false
        /w14311     # pointer truncation
        /w14545     # expression before comma evaluates to a fn missing argument list
        /w14546     # function call before comma missing argument list
        /w14547     # operator before comma has no effect
        /w14549     # operator before comma has no effect; did you mean ;?
        /w14555     # expression has no effect; expected expression with side effect
        /w14619     # pragma warning: there is no warning number 'number'
        /w14640     # thread un-safe static member init
        /w14826     # conversion is sign-extended
        /w14905     # wide string literal cast to LPSTR
        /w14906     # string literal cast to LPWSTR
        /w14928     # illegal copy-initialization
        /permissive- # strict standard conformance
    )

    set(CLANG_WARNINGS
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wimplicit-fallthrough
    )

    set(GCC_WARNINGS
        ${CLANG_WARNINGS}
        -Wmisleading-indentation
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
        -Wuseless-cast
    )

    if(RTS_WARNINGS_AS_ERRORS)
        list(APPEND MSVC_WARNINGS  /WX)
        list(APPEND CLANG_WARNINGS -Werror)
        list(APPEND GCC_WARNINGS   -Werror)
    endif()

    if(MSVC)
        target_compile_options(${target} INTERFACE ${MSVC_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        target_compile_options(${target} INTERFACE ${CLANG_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target} INTERFACE ${GCC_WARNINGS})
    else()
        message(WARNING "Unknown compiler '${CMAKE_CXX_COMPILER_ID}', no warnings set")
    endif()
endfunction()
