cmake_minimum_required(VERSION 3.9)
project(ffnetwork)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_FILES
        client.cpp
        request.cpp
        request_impl.h
        request_impl.cpp
        request_task_impl.h
        request_task_impl.cpp
        curl_client.h
        curl_client.cpp
        log/logger.h
        log/logger.cpp
        log/log_macro.h
        thread/message_queue.h
        thread/message_queue.cpp
        thread/message_handler.h
        thread/message_handler.cpp
        thread/thread_annotations.h
        thread/critical_section.h
        thread/critical_section.cpp
)

add_library(ffnetwork 
    "${FFNETWORK_INCLUDE_DIRECTORY}/ffnetwork/client.h"
    "${FFNETWORK_INCLUDE_DIRECTORY}/ffnetwork/request_task.h"
    "${FFNETWORK_INCLUDE_DIRECTORY}/ffnetwork/request_task_delegate.h"
    "${FFNETWORK_INCLUDE_DIRECTORY}/ffnetwork/request.h"
    "${FFNETWORK_INCLUDE_DIRECTORY}/ffnetwork/response.h"
    ${SOURCE_FILES}
)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(ffnetwork PUBLIC -DDebug)
endif()

target_include_directories(ffnetwork PUBLIC "${FFNETWORK_INCLUDE_DIRECTORY}")