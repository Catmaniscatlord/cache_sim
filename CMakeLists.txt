#
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
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enforce a strcit compiler over the whole project
add_compile_options(-Werror -Wall -Wextra -Wconversion)

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
