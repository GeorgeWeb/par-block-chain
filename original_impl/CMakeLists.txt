cmake_minimum_required(VERSION 3.12)
project(block_chain)

set(CMAKE_CXX_STANDARD 17)

# pass the source files to compile and produce an executable from them
set(SOURCES src/main.cpp src/sha256.cpp src/${PROJECT_NAME}.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})
