/**
 * filename: cache_sim.cpp
 *
 * description: loads config and trace files into the cache simulator
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

#include "cache_sim.hpp"
#include <cstdint>

void CacheSim::RunSimulation()
{
	CacheWrapper cache_wrapper;
	if (cache_conf_.miss_penalty)
		cache_wrapper.set_cache(Cache<true>{cache_conf_});
	else
		cache_wrapper.set_cache(Cache<false>{cache_conf_});

	// memory access count
	const auto tc = stack_trace_.size();

	uint64_t rc{}; // read count
	uint64_t wc{}; // write count
	uint64_t rm{}; // read misses
	uint64_t wm{}; // write misses
	uint64_t rt{}; // run time

	for (auto &memory_access : stack_trace_)
	{
		if (!cache_wrapper.AccessMemory(memory_access.address, memory_access.is_read))
		{
			if (memory_access.is_read)
			{
				rc++;
				rm += memory_access.last_memory_access_count;
			}
			else
			{
				wc++;
				wm += memory_access.last_memory_access_count;
			}
		}
	}

	results_.total_hit_rate = static_cast<float>(rm + wm) / tc;
	results_.read_hit_rate = static_cast<float>(rm) / rc;
	results_.write_hit_rate = static_cast<float>(wm) / wc;
	results_.run_time = rt;
}

// Least Recently Used
template <bool is_lru>
bool Cache<is_lru>::AccessMemory(address_t address, bool is_read)
	requires is_lru
{
	bool miss = false;
	const uint_fast8_t index = (address >> offset_size_) & ((1 << (index_size_ + 1)) - 1);

	cache_block_t block_id{address, is_read, tag_size_};
	// not in cache
	if (cache_[index].map.find(block_id) == cache_[index].map.end())
	{
		miss = true;
		// list is full
		if (cache_[index].list.size() == cache_set_size_)
		{
			// remove the element from the back of the list
			// from the map
			cache_[index].map.erase(cache_[index].list.back());
			cache_[index].list.pop_back();
		}
	}
	else
		cache_[index].list.erase(cache_[index].map[block_id]);

	cache_[index].list.push_front(block_id);
	cache_[index].map.insert(std::make_pair(block_id, cache_[index].list.begin()));

	return miss;
};
