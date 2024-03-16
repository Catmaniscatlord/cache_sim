/**
 * filename: cache_factory.hpp
 *
 * description: header file for the cache factory
 *
 * authors: Chamberlain, David
 **/

#pragma once

#include <memory>

#include "cache.hpp"

namespace CacheFactory
{
std::unique_ptr<CacheBase> CreateCache(const CacheConf &cc);
};
