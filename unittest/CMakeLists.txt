cmake_minimum_required(VERSION 3.9)

project(ffnetwork)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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


