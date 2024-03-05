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
#

cmake_minimum_required(VERSION 3.20)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# to fix clang errors
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(CacheSim)
add_subdirectory(src)

# ##############################################################################
# FETCH PACKAGES #
# ##############################################################################

set(BOOST_INCLUDE_LIBRARIES program_options unordered)
set(BOOST_ENABLE_CMAKE ON)
FetchContent_Declare(
  Boost
  URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.gz
      USES_TERMINAL_DOWNLOAD
      TRUE
  DOWNLOAD_NO_EXTRACT FALSE)

FetchContent_Declare(
  matplotplusplus
  URL https://github.com/alandefreitas/matplotplusplus/archive/refs/tags/v1.2.1.tar.gz
      USES_TERMINAL_DOWNLOAD
      TRUE
  DOWNLOAD_NO_EXTRACT FALSE)
FetchContent_MakeAvailable(Boost matplotplusplus)

# ##############################################################################
# Project Wide Options #
# ##############################################################################

# This is AFTER the fetch contents otherwise we will compile the packages we
# include with our strick compiler settings.

# Enforce a strict compiler over the whole project
add_compile_options(-Werror -Wall -Wextra -Wconversion -Wpedantic)
