cmake_minimum_required(VERSION 3.9)

project(ffnetwork)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

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

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(FFNETWORK_INCLUDE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/output")
message("OUTPUT_DIRECTORY: ${OUTPUT_DIRECTORY}")

add_subdirectory(source)
add_subdirectory(3rdparty)
add_subdirectory(command)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  add_subdirectory(unittest)
endif()
