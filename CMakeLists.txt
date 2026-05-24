# =============================================================================
#  RTS Engine - root CMake configuration
# =============================================================================
cmake_minimum_required(VERSION 3.23)
 
project(rts_engine
    VERSION 0.1.0
    DESCRIPTION "Real-Time RTS Engine with authoritative server architecture"
    LANGUAGES CXX
)
 
# -----------------------------------------------------------------------------
#  Global settings
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
 
# Export compile_commands.json for clang-tidy / IDE integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
 
# Default to Release if user did not specify
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()
 
# Place all built artifacts in predictable locations
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
 
# -----------------------------------------------------------------------------
#  Custom CMake modules
# -----------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
 
include(CompilerWarnings)
include(Sanitizers)
include(StaticAnalysis)
 
# -----------------------------------------------------------------------------
#  Options
# -----------------------------------------------------------------------------
option(RTS_BUILD_TESTS         "Build unit tests"                ON)
option(RTS_BUILD_SERVER        "Build server executable"         ON)
option(RTS_BUILD_CLIENT        "Build client executable"         ON)
option(RTS_ENABLE_CLANG_TIDY   "Run clang-tidy during build"     OFF)
option(RTS_ENABLE_ASAN         "Enable AddressSanitizer"         OFF)
option(RTS_ENABLE_UBSAN        "Enable UndefinedBehaviorSanitizer" OFF)
option(RTS_ENABLE_TSAN         "Enable ThreadSanitizer"          OFF)
option(RTS_WARNINGS_AS_ERRORS  "Treat compiler warnings as errors" OFF)
 
# -----------------------------------------------------------------------------
#  Apply global compile/link interface (warnings, sanitizers, tidy)
# -----------------------------------------------------------------------------
add_library(rts_project_options INTERFACE)
add_library(rts_project_warnings INTERFACE)
 
rts_set_project_warnings(rts_project_warnings)
rts_enable_sanitizers(rts_project_options)
 
if(RTS_ENABLE_CLANG_TIDY)
    rts_enable_clang_tidy()
endif()
 
# -----------------------------------------------------------------------------
#  Third-party dependencies (resolved by Conan)
# -----------------------------------------------------------------------------
# All find_package() calls go here so each subproject sees them.
find_package(EnTT        CONFIG REQUIRED)
find_package(Boost       REQUIRED COMPONENTS system)
find_package(flatbuffers CONFIG REQUIRED)
find_package(spdlog      CONFIG REQUIRED)
find_package(libpqxx     CONFIG REQUIRED)
find_package(SFML        CONFIG REQUIRED COMPONENTS Graphics Window System Network)
find_package(imgui       CONFIG REQUIRED)
find_package(ImGui-SFML  CONFIG REQUIRED)
find_package(prometheus-cpp CONFIG REQUIRED)
find_package(enet        CONFIG REQUIRED)
 
if(RTS_BUILD_TESTS)
    find_package(Catch2 3 CONFIG REQUIRED)
    enable_testing()
    include(CTest)
endif()
 
# -----------------------------------------------------------------------------
#  Subprojects
# -----------------------------------------------------------------------------
add_subdirectory(common)
 
if(RTS_BUILD_SERVER)
    add_subdirectory(server)
endif()
 
if(RTS_BUILD_CLIENT)
    add_subdirectory(client)
endif()
 
if(RTS_BUILD_TESTS)
    add_subdirectory(tests)
endif()
 
# -----------------------------------------------------------------------------
#  Summary
# -----------------------------------------------------------------------------
message(STATUS "")
message(STATUS "=================== RTS Engine =====================")
message(STATUS "  Version           : ${PROJECT_VERSION}")
message(STATUS "  Build type        : ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard      : ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build server      : ${RTS_BUILD_SERVER}")
message(STATUS "  Build client      : ${RTS_BUILD_CLIENT}")
message(STATUS "  Build tests       : ${RTS_BUILD_TESTS}")
message(STATUS "  clang-tidy        : ${RTS_ENABLE_CLANG_TIDY}")
message(STATUS "  ASan / UBSan / TSan : ${RTS_ENABLE_ASAN} / ${RTS_ENABLE_UBSAN} / ${RTS_ENABLE_TSAN}")
message(STATUS "====================================================")
message(STATUS "")