cmake_minimum_required(VERSION 3.9)
project(ffnetwork)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_FILES 
    Client.cpp
    Request.cpp
    RequestImpl.h
    RequestImpl.cpp
    RequestTaskImpl.h
    RequestTaskImpl.cpp
    log/logger.h
    log/logger.cpp
    log/log_macro.h
)

add_library(ffnetwork 
    "${FFNETWORK_INCLUDE_DIRECTORY}/ffnetwork/Client.h"
    "${FFNETWORK_INCLUDE_DIRECTORY}/ffnetwork/RequestTask.h"
    "${FFNETWORK_INCLUDE_DIRECTORY}/ffnetwork/RequestTaskDelegate.h"
    "${FFNETWORK_INCLUDE_DIRECTORY}/ffnetwork/Request.h"
    "${FFNETWORK_INCLUDE_DIRECTORY}/ffnetwork/Response.h"
    ${SOURCE_FILES}
)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(ffnetwork PUBLIC -DDebug)
endif()

target_include_directories(ffnetwork PUBLIC "${FFNETWORK_INCLUDE_DIRECTORY}")