cmake_minimum_required(VERSION 3.15)


set(PROJECT_NAME  integrate_parallel_tpool)
project(${PROJECT_NAME} C CXX) # project(${PROJECT_NAME} C CXX ASM)


find_package(Boost 1.71.0 COMPONENTS program_options system context fiber REQUIRED)

set(CMAKE_CXX_STANDARD 20)

set(WARNINGS_AS_ERRORS OFF)

set(ENABLE_PVS_STUDIO OFF)

set(ENABLE_UBSan OFF)

set(ENABLE_ASAN OFF)

set(ENABLE_TSan OFF)

set(ENABLE_MSAN OFF)

option(COMPILE_FOLLY "Compile folly executable" OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

if (COMPILE_FOLLY)
    add_subdirectory(additional_folly)
endif ()

include(cmake/CompilerWarnings.cmake)

add_executable(${PROJECT_NAME} src/main.cpp
        options_parser/options_parser.cpp options_parser/options_parser.h
        src/check_file_reading.cpp
        src/functions_to_integrate.cpp
        src/integrate.cpp
        src/task.cpp
        src/thread_pool.cpp
        include/functions_to_integrate.h
        include/integrate.h
        include/timer.h
        include/task.hpp
)


target_include_directories(${PROJECT_NAME} PRIVATE options_parser include tomlplusplus)
find_package(Boost 1.71.0 COMPONENTS program_options system REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} Boost::program_options Boost::system)


find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

add_executable(${PROJECT_NAME}_work_stealing src/main.cpp
        options_parser/options_parser.cpp options_parser/options_parser.h
        src/check_file_reading.cpp
        src/functions_to_integrate.cpp
        src/integrate.cpp
        src/task.cpp
        src/thread_pool.cpp
        src/work_stealing_queue.cpp
        src/work_stealing_thread_pool.cpp
        src/thread_joiner.cpp
        include/functions_to_integrate.h
        include/integrate.h
        include/timer.h
        include/task.hpp
        include/work_stealing_queue.h
        include/work_stealing_thread_pool.h
        include/thread_joiner.h
)

target_include_directories(${PROJECT_NAME}_work_stealing PRIVATE options_parser include tomlplusplus)

target_compile_definitions(${PROJECT_NAME}_work_stealing PRIVATE WORK_STEALING)

target_include_directories(${PROJECT_NAME}_work_stealing PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME}_work_stealing Boost::program_options Boost::system)
target_link_libraries(${PROJECT_NAME}_work_stealing Threads::Threads)

##########################################################
# Fixed CMakeLists.txt part
##########################################################

INSTALL(PROGRAMS
        $<TARGET_FILE:${PROJECT_NAME} alternative_method> # ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
        DESTINATION bin)

# Define ALL_TARGETS variable to use in PVS and Sanitizers
set(ALL_TARGETS ${PROJECT_NAME} alternative_method)

# Include CMake setup
include(cmake/main-config.cmake)
