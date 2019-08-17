cmake_minimum_required(VERSION 3.9)

project(ffnetwork)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

set(SOURCE_FILES 
        main.cpp
        request_impl_test.cpp
        request_test.cpp
        message_queue_test.cpp
        time_tuils_test.cpp event_test.cpp thread_test.cpp gunit.h)

add_executable(unittest ${SOURCE_FILES})
target_link_libraries(unittest gmock_main ffnetwork)

target_include_directories(unittest
        PRIVATE
        "${FFNETWORK_INCLUDE_DIRECTORY}"
        ${CMAKE_CURRENT_LIST_DIR}/../source
)

add_test(NAME ffnetwork_unittest COMMAND unittest)

if(CODE_COVERAGE)
  message("Using Code Coverage")
  include(CodeCoverage)
  APPEND_COVERAGE_COMPILER_FLAGS()
  if(CMAKE_COMPILER_IS_GNUCXX)
    SETUP_TARGET_FOR_COVERAGE_GCOVR_HTML(NAME ffnetwork_coverage
          EXECUTABLE ctest -j 3
          DEPENDENCIES unittest)
  else()
    SETUP_TARGET_FOR_COVERAGE_LCOV(NAME ffnetwork_coverage
            EXECUTABLE ctest -j 3
            DEPENDENCIES unittest)
  endif()
endif()

