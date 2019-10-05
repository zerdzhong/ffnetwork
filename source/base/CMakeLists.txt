cmake_minimum_required(VERSION 3.9)
project(ffbase)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FFBASE_SOURCE_FILES
        macros.h
        message.h
        logging.h
        message.cpp
        logging.cpp
        time/time_point.cpp
        time/time_point.h
        time/time_delta.h
        thread/thread_annotations.h
        thread/mutex.cpp
        thread/mutex.h)
    
set(FFBASE_HEADER_FILES)

add_library(ffbase ${FFBASE_SOURCE_FILES} ${FFBASE_HEADER_FILES})

if(CMAKE_GENERATOR MATCHES Xcode)
    include(XcodeSourceGroup)
    SOURCE_GROUP_BY_FOLDER(FFBASE)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(ffbase PUBLIC -DDebug)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_compile_definitions(ffnetwork PRIVATE WIN)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    target_compile_definitions(ffnetwork PRIVATE ANDROID)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_compile_definitions(ffnetwork PRIVATE LINUX)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_compile_definitions(ffnetwork PRIVATE MAC)
endif ()

