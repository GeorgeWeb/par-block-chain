cmake_minimum_required(VERSION 3.12)
project(block_chain)

# find the system threading library,
# prefer the pthreads implementation
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
# determine the threading library
find_package(Threads REQUIRED)
# find openmp
find_package(OpenMP)
# set c/c++ compiler flags for openmp
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

set(CMAKE_CXX_STANDARD 17)

# pass the source files to compile and produce an executable from them
set(SOURCES src/main.cpp src/sha256.cpp src/${PROJECT_NAME}.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

# link the executable with the threading library's generated imported target
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
