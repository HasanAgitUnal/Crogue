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

# Linking

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
        add_executable(${TESTBIN} tests/test_main.cpp src/logic.cpp)
        target_link_libraries(${TESTBIN} PRIVATE GTest::gtest_main)
        add_test(NAME all_tests COMMAND ${TESTBIN})
endif()
