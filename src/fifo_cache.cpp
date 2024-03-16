/**
 * filename: fifo_cache.cpp
 *
 * description: object file for a cache
 *
 * authors: Chamberlain, David
 **/

#include "fifo_cache.hpp"

// Fifo
bool FifoCache::AccessMemory(const address_t& address, const bool& is_read)
{
	bool hit{true};

	const auto index{get_index(address)};

	const auto it{cache_[index].map.find(cache_block_t{address, is_read})};

	// not in cache
	if (it == cache_[index].map.end())
	{
		hit = false;

		// if we have a miss a write with a no-write allocate cache then we
		// return here without adding the block to the cache
		if (!is_read && !is_write_allocate_)
			return hit;

		// map is full
		if (cache_[index].map.size() == associativity_)
			// remove the element from the back of the map
			cache_[index].map.erase(std::prev(cache_[index].list.end()));
		// add the block address to the map, this overwrites the first element
		// if full
		cache_[index].list.push_front({address, is_read});
		cache_[index].map.insert(cache_[index].list.begin());
	}

	return hit;
};
