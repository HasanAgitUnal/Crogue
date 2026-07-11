#
# Config
#

set(APPNAME crogue)

#
# Auto-settings
#

project(${APPNAME} VERSION 1.0)
cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(TESTBIN ${APPNAME}-test)

option(TEST_ENABLED "Enable Tests" OFF)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

#
# Building
#

file(GLOB_RECURSE SOURCE_FILES "src/*.c" "src/*.cpp")

add_executable(${APPNAME} ${SOURCE_FILES})

# -I
target_include_directories(${APPNAME} PRIVATE "${CMAKE_SOURCE_DIR}/include")

# Config
target_compile_definitions(${APPNAME} PRIVATE $<$<CONFIG:Debug>:DEBUG>)
target_compile_definitions(${APPNAME} PRIVATE SOL_LUA_VERSION=504)

# Libs
find_package(Lua 5.4 EXACT REQUIRED)
target_include_directories(${APPNAME} PRIVATE ${LUA_INCLUDE_DIR})

find_package(PkgConfig REQUIRED)

pkg_check_modules(NCURSESW REQUIRED IMPORTED_TARGET ncursesw)

# Precompiled Headers
target_precompile_headers(${APPNAME} PRIVATE
    <nlohmann/json.hpp>
    <sol/sol.hpp>
)

#
# Linking
#

if(UNIX)
        target_link_options(${APPNAME} PRIVATE -rdynamic)
endif()

target_link_libraries(${APPNAME} PRIVATE 
        ${LUA_LIBRARIES} 
        PkgConfig::NCURSESW
)

#
# Test
#

if(TEST_ENABLED)
        enable_testing()
        find_package(GTest REQUIRED)
        include(GoogleTest)
        file(GLOB_RECURSE TEST_SOURCE_FILES "test/*.c" "test/*.cpp")

        if(TEST_SOURCE_FILES)
                add_executable(${TESTBIN} ${TEST_SOURCE_FILES})
                target_link_libraries(${TESTBIN} PRIVATE GTest::gtest_main)
                add_test(NAME all_tests COMMAND ${TESTBIN})
        else()
                message(WARNING "Testing enabled but no test file found under test/ directory")
        endif()
endif()
