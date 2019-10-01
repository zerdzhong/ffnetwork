cmake_minimum_required(VERSION 3.9)
project(ffbase)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FFBASE_SOURCE_FILES message.cpp)
set(FFBASE_HEADER_FILES macros.h message.h)

add_library(ffbase ${FFBASE_SOURCE_FILES} ${FFBASE_HEADER_FILES})

if(CMAKE_GENERATOR MATCHES Xcode)
    include(XcodeSourceGroup)
    SOURCE_GROUP_BY_FOLDER(FFBASE)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(ffbase PUBLIC -DDebug)
endif()
