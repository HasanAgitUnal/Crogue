#
# Settings
#

cmake_minimum_required(VERSION 3.19)

set(APPNAME crogue)
set(APPVER 0.1)

project(${APPNAME} VERSION ${APPVER})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(TESTBIN ${APPNAME}-test)
option(TEST_ENABLED "Enable Tests" OFF)

set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

# check for CCache
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
        message(STATUS "CCache found and using it")
        # config
        set(ENV{CCACHE_SLOPPINESS} "time_macros,pch_defines,file_stat_matches,file_stat_matches_ctime")
        set(ENV{CCACHE_PCH_EXTERNAL_CHECKSUM} "true")
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
        set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
else()
        message(STATUS "CCache not found")
endif()

# check for LLD
find_program(LLD_PROGRAM ld.lld)
if(LLD_PROGRAM)
        message(STATUS "LLD found and using it")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld")
else()
        message(STATUS "LLD not found")
endif()

# enable LTO
#include(CheckIPOSupported)
#check_ipo_supported(RESULT ipo_supported)
#if(ipo_supported)
#        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
#        message(STATUS "LTO enabled")
#else()
#        message(STATUS "LTO not supported")
#endif()

#
# Building
#

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "src/*.c" "src/*.cpp" "src/*.cxx")
file(GLOB THIRD_PARTY LIST_DIRECTORIES true CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/*/include")

add_executable(${APPNAME} ${SOURCE_FILES})

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16)
        target_compile_options(${APPNAME} PRIVATE -Wno-template-body)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19)
        target_compile_options(${APPNAME} PRIVATE -Wno-unknown-warning-option -Wno-template-body)
endif()

# include dirs
target_include_directories(${APPNAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${THIRD_PARTY}
)

# definitions
target_compile_definitions(${APPNAME} PRIVATE $<$<CONFIG:Debug>:DEBUG>)

#
# Libs
#

find_package(PkgConfig REQUIRED)

# lua 5.1-5.4
set(_lua_pkg_names lua54 lua5.4 lua-5.4 lua53 lua5.3 lua-5.3 lua52 lua5.2 lua-5.2 lua51 lua5.1 lua-5.1)
set(_lua_selected "")

foreach(_pkg IN LISTS _lua_pkg_names)
        pkg_check_modules(_LUA_SEARCH QUIET ${_pkg})
        if(_LUA_SEARCH_FOUND AND _LUA_SEARCH_VERSION VERSION_GREATER_EQUAL 5.1 AND _LUA_SEARCH_VERSION VERSION_LESS 5.5)
                set(_lua_selected ${_pkg})
                break()
        endif()
endforeach()

if(NOT _lua_selected)
        pkg_check_modules(_LUA_SEARCH QUIET lua)
        if(_LUA_SEARCH_FOUND AND _LUA_SEARCH_VERSION VERSION_GREATER_EQUAL 5.1 AND _LUA_SEARCH_VERSION VERSION_LESS 5.5)
                set(_lua_selected lua)
        endif()
endif()

if(NOT _lua_selected)
        message(FATAL_ERROR "No supported Lua 5.1-5.4 pkg-config module found")
endif()

pkg_check_modules(LUA REQUIRED IMPORTED_TARGET ${_lua_selected})

string(REGEX MATCH "^([0-9]+)\\.([0-9]+)" _lua_version_match "${LUA_VERSION}")
math(EXPR SOL_LUA_VERSION "${CMAKE_MATCH_1} * 100 + ${CMAKE_MATCH_2}")
target_compile_definitions(${APPNAME} PRIVATE SOL_LUA_VERSION=${SOL_LUA_VERSION})

# ncursesw
pkg_check_modules(NCURSESW REQUIRED IMPORTED_TARGET ncursesw)

# minizip-ng
find_package(minizip-ng REQUIRED)

#
# Precompiled Headers
#

target_precompile_headers(${APPNAME} PRIVATE
        <nlohmann/json.hpp>
        <sol/sol.hpp>
        <minilog.hpp>
)

#
# Linking
#

target_link_options(${APPNAME} PRIVATE
  $<$<NOT:$<CONFIG:Release>>:-rdynamic>
)

target_link_libraries(${APPNAME} PRIVATE
        PkgConfig::LUA
        PkgConfig::NCURSESW
        MINIZIP::minizip-ng
)

#
# Test
#

if(TEST_ENABLED)
        message(STATUS "Testing is enabled so configuring it")
        enable_testing()
        find_package(GTest REQUIRED)
        include(GoogleTest)

        file(GLOB_RECURSE TEST_SOURCE_FILES CONFIGURE_DEPENDS "test/*.c" "test/*.cpp")

        if(TEST_SOURCE_FILES)
                add_executable(${TESTBIN} ${TEST_SOURCE_FILES})
                target_link_libraries(${TESTBIN} PRIVATE GTest::gtest_main)
                add_test(NAME all_tests COMMAND ${TESTBIN})
        else()
                message(WARNING "Testing enabled but no test file found under test/ directory")
        endif()
endif()
