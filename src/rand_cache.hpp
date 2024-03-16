/**
 * filename: rand_cache.hpp
 *
 * description: header file for a random cache
 *
 * authors: Chamberlain, David
 **/

#pragma once

#include <random>

#include "cache.hpp"
#include "cache_block.hpp"

class RandCache : public Cache<std::vector<cache_block_t>>
{
public:
	RandCache(CacheConf cc) : Cache(cc){};
	bool AccessMemory(const address_t &address, const bool &read) override;

private:
	static std::mt19937 kGen;
};
