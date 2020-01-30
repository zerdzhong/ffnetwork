cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(fftool main.cpp downloader.cpp)

target_include_directories(fftool PRIVATE ../source)

target_link_libraries(fftool ffnetwork)

