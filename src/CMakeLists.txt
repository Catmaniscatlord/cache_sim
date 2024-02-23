cmake_minimum_required(VERSION 3.11)
	
####################
# LIBRARY CREATION #
####################
add_library(libCacheSim STATIC cache_sim.cpp cache_sim.hpp)
target_compile_options(libCacheSim PRIVATE -Wall -Wextra)

####################
# MAIN EXECUTEABLE #
####################
add_executable(Main main.cpp)

target_link_libraries(Main libCacheSim)

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
