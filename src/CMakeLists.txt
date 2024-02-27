####
# filename: CMakeLists.txt
#
# description: Build file
#
# authors: Chamberlain, David
#
# class: CSE 3031
# instructor: Zheng
# assignment: Lab #2
#
# assigned: 2/22/2024
# due: 3/7/2024
#
####

cmake_minimum_required(VERSION 3.11)
	
####################
# LIBRARY CREATION #
####################
add_library(libCacheSim STATIC cache_sim.cpp cache_sim.hpp)
target_link_libraries(libCacheSim Boost::unordered)

####################
# MAIN EXECUTEABLE #
####################
add_executable(Main main.cpp util.hpp util.cpp)

target_link_libraries(Main libCacheSim Boost::program_options)

###########
# TESTING #
###########
add_executable(CacheSimTest cache_sim_test.cpp)
target_link_libraries(CacheSimTest gtest_main libCacheSimTest)

enable_testing()
add_test(NAME CacheSimTest COMMAND CacheSimTest)

set_target_properties(
	CacheSimTest
	PROPERTIES
	EXCLUDE_FROM_ALL TRUE)
