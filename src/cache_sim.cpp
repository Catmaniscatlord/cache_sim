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
#include <iterator>

void CacheSim::RunSimulation()
{
	// memory access count
	const auto tc = stack_trace_ref_->size();

	uint64_t rc{};	// read count
	uint64_t wc{};	// write count
	uint64_t rm{};	// read misses
	uint64_t wm{};	// write misses
	uint64_t rt{};	// run time
	uint64_t at{};	// access time

	for (auto &ma : *stack_trace_ref_)
	{
		if (ma.is_read)
			rc++;
		else
			wc++;
		rt += ma.last_memory_access_count;

		// if miss
		if (!cache_wrapper_.AccessMemory(ma.address, ma.is_read))
		{
			rt += cache_conf_.miss_penalty;
			at += cache_conf_.miss_penalty;
			if (ma.is_read)
				rm++;
			else
				wm++;
		}
		else
		{
			// if hit
			rt++;
			at++;
		}
	}

	results_.total_hit_rate =
		1.0f - static_cast<double>(rm + wm) / static_cast<double>(tc);
	results_.read_hit_rate =
		1.0f - static_cast<double>(rm) / static_cast<double>(rc);
	results_.write_hit_rate =
		1.0f - static_cast<double>(wm) / static_cast<double>(wc);
	results_.run_time = rt;
	results_.average_memory_access_time =
		static_cast<double>(at) / static_cast<double>(tc);
}

// Least Recently Used
template <bool is_lru>
bool Cache<is_lru>::AccessMemory(address_t address, bool is_read)
requires is_lru
{
	bool hit = true;

	const uint_fast8_t index = static_cast<uint_fast8_t>(
		(address >> offset_size_) & ((1 << index_size_) - 1));

	const auto it = cache_[index].map.find(cache_block_t{address, is_read});

	// not in cache
	if (it == cache_[index].map.end())
	{
		hit = false;

		// if we have a miss a write with a no-write allocate cache then we
		// return here without adding the block to the cache
		if (!is_read && !is_write_allocate_)
			return hit;

		// map is full
		if (cache_[index].list.size() == cache_set_size_)
		{
			// remove the element from the back of the list and from the map
			cache_[index].map.erase(std::prev(cache_[index].list.end()));
			cache_[index].list.pop_back();
		}
		// add the block address to the map
		cache_[index].list.emplace_front(address, is_read);
		cache_[index].map.insert(cache_[index].list.begin());
	}
	else
		cache_[index].list.splice(
			cache_[index].list.begin(), cache_[index].list, *it);

	return hit;
};

// random replacement cache
template <bool is_lru>
bool Cache<is_lru>::AccessMemory(address_t address, bool is_read)
requires (not is_lru)
{
	bool hit = true;
	const uint_fast8_t index = static_cast<uint_fast8_t>(
		(address >> offset_size_) & ((1 << index_size_) - 1));

	// not in cache
	if (!cache_[index].map.contains(cache_block_t{address, is_read}))
	{
		hit = false;
		// if we have a miss a write with a no-write allocate cache
		// then we return here without adding the block to the cache
		if (!is_read && !is_write_allocate_)
			return hit;

		// list is full
		if (cache_[index].map.size() == cache_set_size_)
		{
			std::uniform_int_distribution<std::size_t> dist(
				0, cache_[index].list.size() - 1);

			// remove the random block in the cache
			cache_[index].map.erase(
				std::next(cache_[index].list.begin(), dist(kGen)));
		}
		// put the new block into the cache
		cache_[index].list.emplace_back(address, is_read);
		cache_[index].map.emplace(std::prev(cache_[index].list.end()));
	}

	return hit;
};

template <bool is_lru>
std::mt19937 Cache<is_lru>::kGen(std::random_device{}());
