cmake_minimum_required(VERSION 3.15)


set(PROJECT_NAME integrate_serial)
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

##### main
add_executable(${PROJECT_NAME} main.cpp
        options_parser/options_parser.cpp options_parser/options_parser.h
        src/check_file_reading.cpp
        src/functions_to_integrate.cpp
        src/integrate.cpp
        src/additional_integral.cpp
        src/precise_addition.cpp
        include/functions_to_integrate.h
        include/integrate.h
        include/timer.h
        include/additional_integral.h
        include/precise_addition.h
)

add_executable(additional_precision src/main_precise.cpp
        include/precise_addition.h
        src/precise_addition.cpp
        src/functions_to_integrate_precise.cpp
        include/integrate_precise.h
        src/integrate_precise.cpp
        src/main_precise.cpp
        options_parser/options_parser.cpp options_parser/options_parser.h
        src/check_file_reading.cpp
        include/timer.h
        include/additional_integral.h
)

target_include_directories(${PROJECT_NAME} PRIVATE options_parser include tomlplusplus)
target_include_directories(additional_precision PRIVATE options_parser include tomlplusplus additional_precision additional_precision/precise_implementation)

find_package(Boost 1.71.0 COMPONENTS program_options system REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIR})
target_include_directories(additional_precision PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} Boost::program_options Boost::system)
target_link_libraries(additional_precision Boost::program_options Boost::system)

##### gsl
find_package(GSL REQUIRED)

add_executable(gsl_prog
        ./src/gsl/main.cpp
        ./src/check_file_reading.cpp
        ./src/functions_to_integrate.cpp
        ./options_parser/options_parser.cpp)


target_include_directories(gsl_prog PRIVATE ${GSL_INCLUDE_DIRS} options_parser include tomlplusplus)
target_include_directories(gsl_prog PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(gsl_prog PRIVATE 
    ${GSL_LIBRARIES}
    m
    Boost::program_options 
    Boost::system
)

##### monte-carlo
add_executable(monte-carlo
                ./src/additional_integral.cpp
                ./src/precise_addition.cpp

                ./src/monte-carlo/main.cpp
                ./src/integrate.cpp
                ./options_parser/options_parser.cpp
                ./src/check_file_reading.cpp
                ./src/functions_to_integrate.cpp)

target_include_directories(monte-carlo PRIVATE options_parser include tomlplusplus)

target_include_directories(monte-carlo PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(monte-carlo Boost::program_options Boost::system)

##########################################################
# Fixed CMakeLists.txt part
##########################################################

INSTALL(PROGRAMS
        $<TARGET_FILE:${PROJECT_NAME}> # ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
        $<TARGET_FILE:additional_precision>
        DESTINATION bin)

# Define ALL_TARGETS variable to use in PVS and Sanitizers
set(ALL_TARGETS ${PROJECT_NAME})

# Include CMake setup
include(cmake/main-config.cmake)
