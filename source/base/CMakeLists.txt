cmake_minimum_required(VERSION 3.9)
project(ffbase)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "-x objective-c")

set(FFBASE_SOURCE_FILES
        macros.h
        messageloop/message.h
        logging.h
        messageloop/message.cpp
        logging.cpp
        time/time_point.cpp
        time/time_point.h
        time/time_delta.h
        thread/thread_annotations.h
        thread/mutex.cpp
        thread/mutex.h
        messageloop/task_runner.cpp
        messageloop/task_runner.h
        messageloop/message_loop.cpp
        messageloop/message_loop.h
        messageloop/message_loop_impl.cpp
        messageloop/message_loop_impl.h
        messageloop/message_loop_task_queues.cpp
        messageloop/message_loop_task_queues.h
        messageloop/delayed_task.cpp
        messageloop/delayed_task.h
        thread/thread_local.h
        thread/thread_local.cpp)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(FFBASE_SOURCE_FILES ${FFBASE_SOURCE_FILES}
            messageloop/message_loop_impl_darwin.h
            messageloop/message_loop_impl_darwin.mm
            platform/darwin/cf_reference_utils.mm
            platform/darwin/cf_reference_utils.h)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(FFBASE_SOURCE_FILES ${FFBASE_SOURCE_FILES}
            messageloop/message_loop_impl_linux.h
            messageloop/message_loop_impl_linux.cpp)
endif ()
    
set(FFBASE_HEADER_FILES )

add_library(ffbase ${FFBASE_SOURCE_FILES} ${FFBASE_HEADER_FILES})

if(CMAKE_GENERATOR MATCHES Xcode)
    include(XcodeSourceGroup)
    SOURCE_GROUP_BY_FOLDER(FFBASE)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(ffbase PUBLIC -DDebug)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_compile_definitions(ffbase PRIVATE WIN)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    target_compile_definitions(ffbase PRIVATE ANDROID)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_compile_definitions(ffbase PRIVATE LINUX)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    find_library(CoreFoundationLib CoreFoundation)

    if (NOT CoreFoundationLib)
        message(FATAL_ERROR "CoreFoundation not found")
    endif ()

    target_link_libraries(ffbase PRIVATE ${CoreFoundationLib})
    target_compile_definitions(ffbase PRIVATE MAC)
endif ()

target_include_directories(ffbase PUBLIC ${CMAKE_CURRENT_LIST_DIR})
