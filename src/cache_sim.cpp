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

void CacheSimulator::RunSimulation()
{
	// memory access count
	const auto ac{stack_trace_ref_->size()};

	uint64_t rc{};	// read count
	uint64_t wc{};	// write count
	uint64_t ic{};	// instruction count
	uint64_t rm{};	// read misses
	uint64_t wm{};	// write misses

	for (const auto& ma : *stack_trace_ref_)
	{
		if (ma.is_read)
			rc++;
		else
			wc++;
		ic += ma.last_memory_access_count + 1;

		// if miss
		if (!cache_wrapper_.AccessMemory(ma.address, ma.is_read))
		{
			if (ma.is_read)
				rm++;
			else
				wm++;
		}
	}

	results_.total_hit_rate =
		1.0f - static_cast<double>(rm + wm) / static_cast<double>(ac);
	results_.read_hit_rate =
		1.0f - static_cast<double>(rm) / static_cast<double>(rc);
	results_.write_hit_rate =
		1.0f - static_cast<double>(wm) / static_cast<double>(wc);
	results_.run_time = ic + (rm + wm) * cache_conf_.miss_penalty_;
	results_.average_memory_access_time =
		1 + (1.f - results_.total_hit_rate) * cache_conf_.miss_penalty_;
}

// Least Recently Used
template <bool is_fifo>
bool Cache<is_fifo>::AccessMemory(const address_t& address, const bool& is_read)
requires is_fifo
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

// random replacement cache
template <bool is_fifo>
bool Cache<is_fifo>::AccessMemory(const address_t& address, const bool& is_read)
requires (not is_fifo)
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
			std::uniform_int_distribution<std::size_t> dist(
				0, cache_[index].list.size() - 1);

			const auto i{dist(kGen)};
			// remove the random block in the cache
			cache_[index].map.erase(std::next(cache_[index].list.begin(), i));
			cache_[index].list[i] = {address, is_read};
			cache_[index].map.emplace(std::next(cache_[index].list.begin(), i));
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

template <bool is_fifo>
std::mt19937 Cache<is_fifo>::kGen(std::random_device{}());
