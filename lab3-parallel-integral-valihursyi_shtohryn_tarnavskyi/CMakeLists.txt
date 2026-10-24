cmake_minimum_required(VERSION 3.15)


set(PROJECT_NAME integrate_parallel)
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

add_executable(${PROJECT_NAME} main.cpp
        options_parser/options_parser.cpp options_parser/options_parser.h
        src/check_file_reading.cpp
        src/functions_to_integrate.cpp
        src/integrate.cpp
        include/functions_to_integrate.h
        include/integrate.h
        include/timer.h
)

target_include_directories(${PROJECT_NAME} PRIVATE options_parser include tomlplusplus)

find_package(Boost 1.71.0 COMPONENTS program_options system REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} Boost::program_options Boost::system)

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

### MUTEX CONF ###
set(MUTEX_PROJECT "${PROJECT_NAME}_mutex")

add_executable(${MUTEX_PROJECT} ./src/additional_two/main.cpp
        options_parser/options_parser.cpp options_parser/options_parser.h
        src/check_file_reading.cpp
        src/functions_to_integrate.cpp
        src/integrate.cpp
)

target_include_directories(${MUTEX_PROJECT} PRIVATE options_parser include tomlplusplus)
target_include_directories(${MUTEX_PROJECT} PRIVATE ${Boost_INCLUDE_DIR})

target_link_libraries(${MUTEX_PROJECT} Boost::program_options Boost::system)
target_link_libraries(${MUTEX_PROJECT} Threads::Threads)

### MUTEX BAD CONF ###

set(MUTEX_PROJECT_BAD "${PROJECT_NAME}_mutex_bad")

add_executable(${MUTEX_PROJECT_BAD} ./src/additional_three/main_mutex_bad.cpp
        options_parser/options_parser.cpp options_parser/options_parser.h
        src/check_file_reading.cpp
        src/functions_to_integrate.cpp
        src/integrate.cpp
)

target_include_directories(${MUTEX_PROJECT_BAD} PRIVATE options_parser include tomlplusplus)
target_include_directories(${MUTEX_PROJECT_BAD} PRIVATE ${Boost_INCLUDE_DIR})

target_link_libraries(${MUTEX_PROJECT_BAD} Boost::program_options Boost::system)
target_link_libraries(${MUTEX_PROJECT_BAD} Threads::Threads)

### ATOMIC BAD CONF ###

set(MUTEX_ATOMIC_BAD "${PROJECT_NAME}_atomic_bad")

add_executable(${MUTEX_ATOMIC_BAD} ./src/additional_three/main_atomic_bad.cpp
        options_parser/options_parser.cpp options_parser/options_parser.h
        src/check_file_reading.cpp
        src/functions_to_integrate.cpp
        src/integrate.cpp
)

target_include_directories(${MUTEX_ATOMIC_BAD} PRIVATE options_parser include tomlplusplus)
target_include_directories(${MUTEX_ATOMIC_BAD} PRIVATE ${Boost_INCLUDE_DIR})

target_link_libraries(${MUTEX_ATOMIC_BAD} Boost::program_options Boost::system)
target_link_libraries(${MUTEX_ATOMIC_BAD} Threads::Threads)

##########################################################
# Fixed CMakeLists.txt part
##########################################################

INSTALL(PROGRAMS
        $<TARGET_FILE:${PROJECT_NAME}> # ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
        DESTINATION bin)

# Define ALL_TARGETS variable to use in PVS and Sanitizers
set(ALL_TARGETS ${PROJECT_NAME})

# Include CMake setup
include(cmake/main-config.cmake)
