cmake_minimum_required(VERSION 3.10)

project(ffnetwork)

enable_testing()

set(SOURCE_FILES 
    main.cpp
    RequestImplTest.cpp
)

add_executable(unittest ${SOURCE_FILES})
target_link_libraries(unittest gmock_main ffnetwork)

target_include_directories(unittest
        PRIVATE
        "${FFNETWORK_INCLUDE_DIRECTORY}"
        ${CMAKE_CURRENT_LIST_DIR}/../source
)

add_test(NAME ffnetwork_unittest COMMAND unittest)


