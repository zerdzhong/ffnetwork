cmake_minimum_required(VERSION 3.9)
project(ffnetwork)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FFNETWORK_SOURCE_FILES
        net/client.cpp
        net/request.cpp
        net/request_impl.h
        net/request_impl.cpp
        net/request_task_impl.h
        net/request_task_impl.cpp
        net/curl_client.h
        net/curl_client.cpp
        utils/time_utils.h
        utils/time_utils.cpp
        utils/sha256.h
        utils/sha256.cpp
        net/response_impl.h
        net/response_impl.cpp)


set(FFNETWORK_HEADER_FILES
        "${FFNETWORK_INCLUDE_DIRECTORY}/ffnetwork/client.h"
        "${FFNETWORK_INCLUDE_DIRECTORY}/ffnetwork/request_task.h"
        "${FFNETWORK_INCLUDE_DIRECTORY}/ffnetwork/request_task_delegate.h"
        "${FFNETWORK_INCLUDE_DIRECTORY}/ffnetwork/request.h"
        "${FFNETWORK_INCLUDE_DIRECTORY}/ffnetwork/response.h"
        net/url.cpp net/url.h)

add_library(ffnetwork ${FFNETWORK_SOURCE_FILES} ${FFNETWORK_HEADER_FILES})

if (CMAKE_GENERATOR MATCHES Xcode)
    include(XcodeSourceGroup)
    SOURCE_GROUP_BY_FOLDER(FFNETWORK)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_compile_definitions(ffnetwork PRIVATE WIN)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    target_compile_definitions(ffnetwork PRIVATE ANDROID)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_compile_definitions(ffnetwork PRIVATE LINUX)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_compile_definitions(ffnetwork PRIVATE DARWIN)
endif ()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(ffnetwork PUBLIC -DDebug)
endif()

add_subdirectory(base)

target_link_libraries(ffnetwork PUBLIC libcurl ffbase)

target_include_directories(ffnetwork PUBLIC 
    "${FFNETWORK_INCLUDE_DIRECTORY}"
    ${CMAKE_CURRENT_SOURCE_DIR}
)
