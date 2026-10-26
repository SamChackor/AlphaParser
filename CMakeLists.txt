cmake_minimum_required(VERSION 3.14)
project(AlphaParser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimization & warnings
if (MSVC)
    add_compile_options(/W4 /permissive- /O2)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O3)
endif()

# Core static library
add_library(alphaparser_core STATIC
    src/Lexer.cpp
    src/Parser.cpp
    src/SemanticAnalyzer.cpp
    src/CodeGenerator.cpp
    src/Compiler.cpp
)

target_include_directories(alphaparser_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Demo binary
add_executable(alphaparser_demo examples/main.cpp)
target_link_libraries(alphaparser_demo PRIVATE alphaparser_core)
