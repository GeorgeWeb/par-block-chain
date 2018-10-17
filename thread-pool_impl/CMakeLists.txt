cmake_minimum_required(VERSION 3.12)
project(block_chain)

# find the system threading library,
# prefer the pthreads implementation
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
# determine the threading library
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)

# pass the source files to compile and produce an executable from them
set(SOURCES src/main.cpp src/sha256.cpp src/${PROJECT_NAME}.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

# link the executable with the threading library's generated imported target
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
