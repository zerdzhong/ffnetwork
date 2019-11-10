cmake_minimum_required(VERSION 3.9)

project(ffbase)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

set(SOURCE_FILES
        main.cpp
        message_test.cpp
        time_delta_test.cpp
        time_test.cpp
        thread_local_test.cpp
        message_loop_task_queues_test.cpp
        message_loop_test.cpp
        waitable_event_test.cpp
        thread_test.cpp)

add_executable(ffbase_unittest ${SOURCE_FILES})

if (${UNITTEST_TIME_INSENSITIVE})
    target_compile_definitions(ffbase_unittest PRIVATE -DUNITTEST_TIME_INSENSITIVE)
endif ()

target_link_libraries(ffbase_unittest gmock_main ffbase)

target_include_directories(ffbase_unittest
        PRIVATE
        "${FFNETWORK_INCLUDE_DIRECTORY}"
        ${CMAKE_CURRENT_LIST_DIR}/../../
        )

add_test(NAME ffbase_unittest COMMAND ffbase_unittest)