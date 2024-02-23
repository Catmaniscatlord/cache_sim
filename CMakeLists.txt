cmake_minimum_required(VERSION 3.11)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# to fix clang errors
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(CacheSim)
add_subdirectory(src)

##################
# FETCH PACKAGES #
##################
FetchContent_Declare(
	googletest
	URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
	)
FetchContent_MakeAvailable(googletest)
