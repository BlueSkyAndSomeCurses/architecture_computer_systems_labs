cmake_minimum_required(VERSION 3.15)

find_package(folly REQUIRED)
#find_package(Libevent REQUIRED)
find_package(gflags REQUIRED)
find_package(glog REQUIRED)
find_package(fmt REQUIRED)
find_package(double-conversion REQUIRED)
find_package(Boost 1.71.0 COMPONENTS program_options system context fiber REQUIRED)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

add_executable(integrate_parallel_tpool_folly
        ${CMAKE_SOURCE_DIR}/src/main_folly.cpp
        ${CMAKE_SOURCE_DIR}/options_parser/options_parser.cpp ${CMAKE_SOURCE_DIR}/options_parser/options_parser.h
        ${CMAKE_SOURCE_DIR}/src/check_file_reading.cpp
        ${CMAKE_SOURCE_DIR}/src/functions_to_integrate.cpp
        ${CMAKE_SOURCE_DIR}/src/integrate_folly.cpp
        ${CMAKE_SOURCE_DIR}/include/functions_to_integrate.h
        ${CMAKE_SOURCE_DIR}/include/integrate_folly.h
        ${CMAKE_SOURCE_DIR}/include/timer.h
)

target_include_directories(integrate_parallel_tpool_folly PRIVATE
        ${Boost_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/options_parser
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/tomlplusplus
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(integrate_parallel_tpool_folly PRIVATE ${Libevent_LIBRARIES})
target_include_directories(integrate_parallel_tpool_folly PRIVATE ${Boost_INCLUDE_DIR})
target_include_directories(integrate_parallel_tpool_folly PRIVATE options_parser include tomlplusplus)
target_link_libraries(integrate_parallel_tpool_folly PRIVATE folly gflags glog Threads::Threads Boost::program_options Boost::context Boost::fiber Boost::system double-conversion fmt event event_core event_pthreads event_extra)

set_target_properties(integrate_parallel_tpool_folly PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/additional_folly
)