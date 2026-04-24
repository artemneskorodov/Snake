cmake_minimum_required(VERSION 3.28)

project(Snake VERSION 1.0 LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    find_program(CLANGXX_FOUND clang++)
    if(CLANGXX_FOUND)
        set(CMAKE_CXX_COMPILER "clang++" CACHE STRING "C++ compiler" FORCE)
    endif()
endif()

set(INCLUDE_DIR "include")
set(SOURCES_DIR "sources")

file(GLOB INCLUDE "${INCLUDE_DIR}/*")
file(GLOB SOURCES "${SOURCES_DIR}/*")

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} PRIVATE SFML::Graphics)
