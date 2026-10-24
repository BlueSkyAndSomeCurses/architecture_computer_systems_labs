cmake_minimum_required(VERSION 3.15)

#! Check every comment after the "#!"

#! CHANGE YOUR PROJECT NAME
#  It is used as your project's main executable name. 
set(PROJECT_NAME lab1_comparison)
project(${PROJECT_NAME} C CXX) # project(${PROJECT_NAME} C CXX ASM)

set(CMAKE_CXX_STANDARD 20)

##########################################################
# User configurable options of the template
##########################################################
# Note: symbols like WARNINGS_AS_ERRORS in configuration are intentionally variables
# and not CMake options --using options creates too many problems for students.

#! It is a good practice to set "WARNINGS_AS_ERRORS" ON,
#  but sometimes it creates too much trouble, so default is OFF.
set(WARNINGS_AS_ERRORS OFF)

#! Always use PVS Studio while developing.
set(ENABLE_PVS_STUDIO ON)

#! Select appropriate sanitizers.
#  Definitely enable sanitizers while developing.
#  Disable it for the production builds and before submitting for grading.
#  Only one of the Memory (MSAN), Address (ASAN), or Thread (TSan)
#  sanitizers is applicable at the time -- the first defined.

#! UndefinedBehaviorSanitizer (UBSan).
#  Info: https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html
set(ENABLE_UBSan OFF)
#! AddressSanitizer -- detects use after free or after scope exit,
#  memory overflows and leaks.
#  Info: https://github.com/google/sanitizers/wiki/AddressSanitizer
set(ENABLE_ASAN OFF)
#! ThreadSanitizer -- detects data races.
#  Info: https://clang.llvm.org/docs/ThreadSanitizer.html
set(ENABLE_TSan OFF)
#! MemorySanitizer -- detects uninitialized memory reads
#  Info: https://github.com/google/sanitizers/wiki/MemorySanitizer
set(ENABLE_MSAN OFF)

#! Be default -- build release version if not specified otherwise.
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

# Warnings as errors should be imported here -- do not move this line
include(cmake/CompilerWarnings.cmake)

##########################################################
# Project files, packages, libraries and so on
##########################################################

#! Project main executable source compilation

add_executable(cxx_perf_io
        src/task1/main.cpp
        src/task1/program_one.cpp
        options_parser/options_parser.cpp
        src/check_file_reading.cpp
)

add_executable(cxx_perf_conv
        ./options_parser/options_parser.cpp
        ./src/task2/transfer_funcs.cpp
        ./src/task2/productivity_check.cpp
        ./src/task2/program_two.cpp
        ./src/check_file_reading.cpp
        ./src/task1/program_one.cpp
)

# add_executable(cxx_perf_conv_d
#         ./options_parser/options_parser.cpp
#         ./src/additional3/transfer_funcs_d.cpp
#         ./src/task2/productivity_check.cpp
#         ./src/additional3/additional3.cpp
#         ./src/check_file_reading.cpp
#         ./src/task1/program_one.cpp
# )

# add_executable(cxx_bonus_one
#         ./options_parser/options_parser.cpp
#         ./src/bonus_one.cpp
# )

# target_include_directories(cxx_bonus_one PRIVATE options_parser include)

target_include_directories(cxx_perf_io PRIVATE options_parser include include/task1 include/task2)

target_include_directories(cxx_perf_conv PRIVATE ./include ./include/task2 ./options_parser ./include ./include/task1)

# target_include_directories(cxx_perf_conv_d PRIVATE ./include ./include/task2 ./options_parser ./include ./include/task1 ./include/additional3)

find_package(Boost 1.71.0 COMPONENTS program_options system REQUIRED)
target_include_directories(cxx_perf_io PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(cxx_perf_io Boost::program_options Boost::system)

target_include_directories(cxx_perf_conv PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(cxx_perf_conv Boost::program_options Boost::system)

# target_include_directories(cxx_perf_conv_d PRIVATE ${Boost_INCLUDE_DIR})
# target_link_libraries(cxx_perf_conv_d Boost::program_options Boost::system)

# find_package(Qt6 COMPONENTS Core REQUIRED)
# find_package(absl REQUIRED)
# target_link_libraries(cxx_bonus_one Qt6::Core absl::strings Boost::program_options Boost::system)

##########################################################
# Fixed CMakeLists.txt part
##########################################################

INSTALL(PROGRAMS
        $<TARGET_FILE:cxx_perf_io>
        $<TARGET_FILE:cxx_perf_conv>
        # $<TARGET_FILE:cxx_perf_conv_d>
        # $<TARGET_FILE:cxx_bonus_one>
        DESTINATION bin)

# Define ALL_TARGETS variable to use in PVS and Sanitizers
# set(ALL_TARGETS cxx_perf_io cxx_perf_conv cxx_bonus_one)

# Include CMake setup
include(cmake/main-config.cmake)
