cmake_minimum_required(VERSION 3.16)
project(MiniHttp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(src)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/example/CMakeLists.txt")
    add_subdirectory(example)
endif()