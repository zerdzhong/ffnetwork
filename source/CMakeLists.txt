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
        thread/thread.h
        thread/thread.cpp
        utils/time_utils.h
        utils/time_utils.cpp
        utils/sha256.h
        utils/sha256.cpp
        thread/event.cpp
        thread/event.h
        thread/async_invoker.cpp
        thread/async_invoker.h
        construct_macro.h
        thread/async_invoker_inl.h
        sigslot/sigslot.hpp
        thread/callback.h ../include/ffnetwork/response_impl.h response_impl.cpp)

add_library(ffnetwork 
    "${FFNETWORK_INCLUDE_DIRECTORY}/ffnetwork/client.h"
    "${FFNETWORK_INCLUDE_DIRECTORY}/ffnetwork/request_task.h"
    "${FFNETWORK_INCLUDE_DIRECTORY}/ffnetwork/request_task_delegate.h"
    "${FFNETWORK_INCLUDE_DIRECTORY}/ffnetwork/request.h"
    "${FFNETWORK_INCLUDE_DIRECTORY}/ffnetwork/response.h"
    ${SOURCE_FILES}
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_compile_definitions(ffnetwork PRIVATE WIN)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    target_compile_definitions(ffnetwork PRIVATE ANDROID)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_compile_definitions(ffnetwork PRIVATE LINUX)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_compile_definitions(ffnetwork PRIVATE MAC)
endif ()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(ffnetwork PUBLIC -DDebug)
endif()

target_link_libraries(ffnetwork PUBLIC libcurl)

target_include_directories(ffnetwork PUBLIC 
    "${FFNETWORK_INCLUDE_DIRECTORY}"
    ${CMAKE_CURRENT_SOURCE_DIR}
)