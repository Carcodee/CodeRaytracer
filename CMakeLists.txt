cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(Editor)
add_executable("Editor" "Editor/Source/Editor.cpp")

set_target_properties(Editor PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
option(BUILD_APP "Build the App project" OFF)
option(BUILD_EDITOR "Build the Editor project" ON)

set(SOLUTION_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(CORE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Core/Source")
add_subdirectory(${CORE_DIR})

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE Debug)

target_include_directories(Editor PUBLIC
        ${CORE_DIR}
)

target_link_libraries(Editor PUBLIC
        Core
)