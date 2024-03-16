#
# filename: CMakeLists.txt
#
# description: Build file
#
# authors: Chamberlain, David
#
# class: CSE 3031 instructor: Zheng assignment: Lab #2
#
# assigned: 2/22/2024 due: 3/7/2024

cmake_minimum_required(VERSION 3.21)

# ##############################################################################
# LIBRARY CREATION #
# ##############################################################################
add_library(libCacheSim STATIC cache_sim.cpp rand_cache.cpp fifo_cache.cpp
                               cache_factory.cpp)
target_include_directories(libCacheSim PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(libCacheSim Boost::unordered)

# ##############################################################################
# MAIN EXECUTEABLE #
# ##############################################################################
add_executable(Main main.cpp util.hpp util.cpp)

target_link_libraries(Main libCacheSim Boost::program_options matplot)

install(TARGETS Main RUNTIME DESTINATION bin)

# ##############################################################################
# TESTING #
# ##############################################################################
add_executable(CacheSimTest cache_sim_test.cpp)
target_link_libraries(CacheSimTest PRIVATE gtest_main libCacheSim)

add_test(NAME CacheSimTest COMMAND $<TARGET_FILE:CacheSimTest>)
