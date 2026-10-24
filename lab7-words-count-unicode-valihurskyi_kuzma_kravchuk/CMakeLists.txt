cmake_minimum_required(VERSION 3.15)

#! Check every comment after the "#!"

#! CHANGE YOUR PROJECT NAME
#  It is used as your project's main executable name. 
set(PROJECT_NAME countwords_seq)
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
set(ENABLE_PVS_STUDIO OFF)

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
find_package(ICU REQUIRED COMPONENTS uc i18n)
# Повний список ICU components = data, i18n, io, le, lx,
# test, tu and uc.
set(Boost_USE_STATIC_LIBS OFF) # enable dynamic linking
set(Boost_USE_MULTITHREAD ON) # enable multithreading


option(USE_MAP "Use std::map instead of std::unordered_map" OFF)
option(USE_ABSEIL_HASH_MAP "Use abseil::flat_hash_map instead of std::unordered_map" OFF)

if(USE_MAP)
    message(STATUS "Using std::map")
    add_compile_definitions(USE_MAP)
elseif(USE_ABSEIL_HASH_MAP)
    message(STATUS "Using abseil::flat_hash_map")
    find_package(absl REQUIRED)
    add_compile_definitions(USE_ABSEIL_HASH_MAP)
else()
    message(STATUS "Using std::unordered_map")
endif()

find_package(Boost 1.71.0 COMPONENTS program_options system locale REQUIRED)

find_package(LibArchive REQUIRED)

#! Project main executable source compilation
add_executable(${PROJECT_NAME} src/main.cpp
        options_parser/options_parser.cpp options_parser/options_parser.h
        src/check_file_reading.cpp src/config_parser.cpp src/word_counter.cpp
        include/check_file_reading.h include/timer.h include/config_parser.h include/word_counter.h
)

add_executable(${PROJECT_NAME}_mmap src/main_mmap.cpp
        options_parser/options_parser.cpp options_parser/options_parser.h
        src/check_file_reading.cpp src/config_parser.cpp src/word_counter.cpp
        include/check_file_reading.h include/timer.h include/config_parser.h include/word_counter.h
)

add_executable(${PROJECT_NAME}_icu src/main_icu.cpp
        options_parser/options_parser.cpp options_parser/options_parser.h
        src/check_file_reading.cpp src/config_parser.cpp src/word_counter.cpp
        include/check_file_reading.h include/timer.h include/config_parser.h include/word_counter.h
)

#! Put path to your project headers
target_include_directories(${PROJECT_NAME} PRIVATE options_parser)
target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIR} ${ICU_INCLUDE_DIRS} ${LibArchive_INCLUDE_DIR} include)
target_link_libraries(${PROJECT_NAME} Boost::program_options Boost::system Boost::locale Threads::Threads ${ICU_LIBRARIES} ${LibArchive_LIBRARIES})
if (USE_ABSEIL_HASH_MAP)
    target_link_libraries(${PROJECT_NAME} absl::flat_hash_map)
endif()

target_include_directories(${PROJECT_NAME}_mmap PRIVATE options_parser)
target_include_directories(${PROJECT_NAME}_mmap PRIVATE ${Boost_INCLUDE_DIR} ${ICU_INCLUDE_DIRS} ${LibArchive_INCLUDE_DIR} include)
target_link_libraries(${PROJECT_NAME}_mmap Boost::program_options Boost::system Boost::locale Threads::Threads ${ICU_LIBRARIES} ${LibArchive_LIBRARIES})
if (USE_ABSEIL_HASH_MAP)
    target_link_libraries(${PROJECT_NAME}_mmap absl::flat_hash_map)
endif()

target_include_directories(${PROJECT_NAME}_icu PRIVATE options_parser)
target_include_directories(${PROJECT_NAME}_icu PRIVATE ${Boost_INCLUDE_DIR} ${ICU_INCLUDE_DIRS} ${LibArchive_INCLUDE_DIR} include)
target_link_libraries(${PROJECT_NAME}_icu Boost::program_options Boost::system Boost::locale Threads::Threads ${ICU_LIBRARIES} ${LibArchive_LIBRARIES})
if (USE_ABSEIL_HASH_MAP)
    target_link_libraries(${PROJECT_NAME}_icu absl::flat_hash_map)
endif()

#! Add external packages
# options_parser requires boost::program_options library

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
