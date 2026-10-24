cmake_minimum_required(VERSION 3.15)


set(PROJECT_NAME integrate_parallel_queue)
project(${PROJECT_NAME} C CXX) # project(${PROJECT_NAME} C CXX ASM)

set(CMAKE_CXX_STANDARD 20)

set(WARNINGS_AS_ERRORS OFF)

set(ENABLE_PVS_STUDIO OFF)

set(ENABLE_UBSan OFF)

set(ENABLE_ASAN OFF)

set(ENABLE_TSan OFF)

set(ENABLE_MSAN OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

include(cmake/CompilerWarnings.cmake)

add_executable(${PROJECT_NAME} src/main.cpp
        options_parser/options_parser.cpp options_parser/options_parser.h
        src/check_file_reading.cpp
        src/functions_to_integrate.cpp
        src/integrate.cpp
        include/functions_to_integrate.h
        include/integrate.h
        include/timer.h
)

add_executable(alternative_method src/alternative_main.cpp
        options_parser/options_parser.cpp options_parser/options_parser.h
        src/check_file_reading.cpp
        src/functions_to_integrate.cpp
        src/alternative_method.cpp
        include/functions_to_integrate.h
        include/alternative_method.h
        include/timer.h
)

target_include_directories(${PROJECT_NAME} PRIVATE options_parser include tomlplusplus)
target_include_directories(alternative_method PRIVATE options_parser include tomlplusplus)

find_package(Boost 1.71.0 COMPONENTS program_options system REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIR})
target_include_directories(alternative_method PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} Boost::program_options Boost::system)
target_link_libraries(alternative_method Boost::program_options Boost::system)

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)
target_link_libraries(alternative_method Threads::Threads)

### Bonus three ###

add_executable(bonus_three src/bonus_three/main.cpp
        options_parser/options_parser.cpp options_parser/options_parser.h
        src/check_file_reading.cpp
        src/functions_to_integrate.cpp
        src/integrate.cpp
        include/functions_to_integrate.h
        include/integrate.h
        include/timer.h
)

target_include_directories(bonus_three PRIVATE options_parser include tomlplusplus)
target_include_directories(bonus_three PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(bonus_three Boost::program_options Boost::system)
target_link_libraries(bonus_three Threads::Threads)


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
