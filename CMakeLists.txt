cmake_minimum_required(VERSION 3.25)
project(Octocore VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(SOURCES
src/main.cpp
src/Application.cpp
src/Chip8.cpp
)

set(PROJECT_ROOT ${CMAKE_SOURCE_DIR})
add_compile_definitions(PROJECT_ROOT="${PROJECT_ROOT}")

if(MSVC)
	add_compile_options( /W4 /WX-)
else()
	add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

find_package(SDL3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3 glad::glad)