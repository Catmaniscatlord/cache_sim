/**
 * filename: util.hpp
 *
 * description: header file for utility functions
 *
 * authors: Chamberlain, David
 *
 **/

#pragma once

#include <optional>

#include "cache_sim.hpp"

namespace Util
{
std::optional<CacheConf> ReadCacheConfFile(const std::string &s);
std::optional<StackTrace> ReadStackTraceFile(const std::string &s);
}  // namespace Util
