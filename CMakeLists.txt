cmake_minimum_required(VERSION 3.10)

project(ffnetwork)

if(NOT WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
endif()

if(USE_ADDRESS_SANITIZER)
  message("Using Address & Leak Sanitizer")
  set(
    CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -fsanitize=address -g -fno-omit-frame-pointer")
  set(
    CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -g -fno-omit-frame-pointer")
endif()

if(CODE_COVERAGE)
  message("Using Code Coverage")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

set(FFNETWORK_INCLUDE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/output")
message("OUTPUT_DIRECTORY: ${OUTPUT_DIRECTORY}")

add_subdirectory(source)