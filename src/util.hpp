/**
 * filename: util.hpp
 *
 * description: header file for utility functions
 *
 * authors: Chamberlain, David
 *
 * class: CSE 3031
 * instructor: Zheng
 * assignment: Lab #2
 *
 * assigned: 2/22/2024
 * due: 3/7/2024
 *
 **/

#pragma once

#include <optional>

#include "cache_sim.hpp"

namespace Util
{
std::optional<CacheConf> ReadCacheConfFile(const std::string &s);
std::optional<CacheConf> ReadCacheConfFile(std::string &&s);

std::optional<StackTrace> ReadStackTraceFile(const std::string &s);
std::optional<StackTrace> ReadStackTraceFile(std::string &&s);
}  // namespace Util
