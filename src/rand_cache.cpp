/**
 * filename: rand_cache.cpp
 *
 * description: object file for a cache
 *
 * authors: Chamberlain, David
 **/

#include "rand_cache.hpp"

bool RandCache::AccessMemory(const address_t& address, const bool& is_read)
{
	bool hit{true};
	const auto index{get_index(address)};

	// not in cache
	if (!cache_[index].map.contains(cache_block_t{address, is_read}))
	{
		hit = false;
		// if we have a miss a write with a no-write allocate cache
		// then we return here without adding the block to the cache
		if (!is_read && !is_write_allocate_)
			return hit;

		// list is full
		if (cache_[index].map.size() == associativity_)
		{
			size_t i;
			if (associativity_ == 0)
				i = 0;
			else
			{
				std::uniform_int_distribution<std::size_t> dist(
					0, cache_[index].list.size() - 1);

				i = static_cast<size_t>(dist(kGen));
			}
			// remove the random block in the cache
			cache_[index].map.erase(
				std::next(cache_[index].list.begin(), static_cast<long>(i)));
			cache_[index].list[i] = {address, is_read};
			cache_[index].map.emplace(
				std::next(cache_[index].list.begin(), static_cast<long>(i)));
		}
		else
		{
			// put the new block into the cache
			cache_[index].list.emplace_back(address, is_read);
			cache_[index].map.emplace(std::prev(cache_[index].list.end()));
		}
	}

	return hit;
};

std::mt19937 RandCache::kGen(std::random_device{}());
