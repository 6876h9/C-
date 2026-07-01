cmake_minimum_required(VERSION 3.10)
project(cminus)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

add_executable(cminus
    main.cpp
    lexer.cpp
    parser.cpp
    typechecker.cpp
    codegen.cpp
    driver.cpp
)

target_include_directories(cminus PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
