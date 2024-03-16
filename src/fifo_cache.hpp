/**
 * filename: rand_cache.hpp
 *
 * description: header file for a random cache
 *
 * authors: Chamberlain, David
 **/

#pragma once

#include <boost/circular_buffer.hpp>

#include "cache.hpp"

class FifoCache : public Cache<boost::circular_buffer<cache_block_t>>
{
public:
	FifoCache(CacheConf cc) : Cache(cc){};
	bool AccessMemory(const address_t &address, const bool &read) override;
};
