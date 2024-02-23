/**
 * filename: cache_sim.hpp
 *
 * description: header file for or cache simulator
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

#include <cmath>
#include <cstdint>
#include <list>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>

using address_t = uint32_t;

struct CacheConf
{
	uint_fast8_t block_size;
	uint_fast8_t associativity;
	uint_fast8_t miss_penalty;
	uint_fast16_t cache_size;
	/**
	 * false : no-write allocate, write-through
	 * true : write-allocate, write-back
	 */
	bool write_allocate;
	/**
	 * false : random Replacement
	 * true : FIFO replacement
	 */
	bool replacement_policy;
};

struct Results
{
	float total_hit_rate;
	float read_hit_rate;
	float write_hit_rate;
	uint64_t run_time;
};

struct MemoryAccess
{
	address_t address;

	// the maximum value of this in the files is 92. 8 bits should be enough
	// to store
	uint_fast8_t last_memory_access_count;

	/**
	 * true : read
	 * false : write
	 **/
	bool is_read;
};

typedef std::vector<MemoryAccess> StackTrace;

struct cache_block_t
{
	address_t block_address;
	// the dirty bit and its functionality is useless
	// unless he gives some clock times for write-through
	// and write-back
	bool dirty;
	uint_fast8_t tag_size;

	bool operator==(const cache_block_t &other) const
	{
		return block_address == other.block_address;
	}
};

// Create a specific hash function for a cache block
template <>
struct std::hash<cache_block_t>
{
	size_t operator()(const cache_block_t &cb) const noexcept
	{

		return (cb.block_address >> (32 - cb.tag_size));
	}
};

template <bool is_lru>
struct CacheIndex
{
	std::list<cache_block_t> list;

	using BlockMap = std::conditional_t<is_lru,
										std::unordered_map<cache_block_t, decltype(list.begin())>,
										std::unordered_set<cache_block_t>>;

	BlockMap map;

	CacheIndex(address_t cache_set_size)
	{
		map.reserve(cache_set_size);
	};
};

template <bool is_lru>
class Cache
{
public:
	Cache() = default;

	Cache(CacheConf cache_conf)
	{
		cache_size_ = cache_conf.cache_size;
		associativity_ = cache_conf.associativity;
		block_size_ = cache_conf.block_size;
		is_write_allocate_ = cache_conf.write_allocate;
		cache_set_size_ = (cache_conf.cache_size / cache_conf.associativity) / cache_conf.block_size,
		offset_size_ = log2(block_size_);
		index_size_ = log2(associativity_);
		tag_size_ = 32 - offset_size_ - index_size_;
		std::fill(cache_.begin(), cache_.end(), CacheIndex<is_lru>{cache_set_size_});
	};

	/**
	 * @brief returns true on hit, false on miss
	 * Two different instantiations for when
	 * the cacche uses random vs LRU for replaccement
	 **/
	bool AccessMemory(address_t address, bool read)
		requires is_lru;
	bool AccessMemory(address_t address, bool read)
		requires(not is_lru);

private:
	address_t cache_size_;
	uint_fast8_t associativity_;
	uint_fast8_t block_size_;
	address_t cache_set_size_;
	uint_fast8_t offset_size_;
	uint_fast8_t index_size_;
	uint_fast8_t tag_size_;

	std::vector<CacheIndex<is_lru>> cache_;

	/**
	 * false : no-write allocate, write-through
	 * true : write-allocate, write-back
	 */
	bool is_write_allocate_;
};

/**
 * @brief Template instantiations must be constexpr
 * So we create a wrapper around both types of
 * caches so that we can access both with a single
 * variable.
 **/
class CacheWrapper
{
public:
	CacheWrapper(){};

	/**
	 * @brief returns true on hit, false on miss
	 **/
	auto AccessMemory(address_t address, bool read)
	{
		if (is_lru_)
			return lru_cache_.AccessMemory(address, read);
		else
			return random_cache_.AccessMemory(address, read);
	}

	// Move semantics :)
	template <bool is_lru>
		requires is_lru
	void set_cache(const Cache<is_lru> &cache)
	{
		is_lru_ = is_lru;
		lru_cache_ = std::move(cache);
	};

	template <bool is_lru>
		requires(not is_lru)
	void set_cache(const Cache<is_lru> &cache)
	{
		is_lru_ = is_lru;
		random_cache_ = std::move(cache);
	};

	template <bool is_lru>
		requires is_lru
	void set_cache(Cache<is_lru> &&cache)
	{
		is_lru_ = is_lru;
		lru_cache_ = std::move(cache);
	};

	template <bool is_lru>
		requires(not is_lru)
	void set_cache(Cache<is_lru> &&cache)
	{
		is_lru_ = is_lru;
		random_cache_ = std::move(cache);
	};

private:
	bool is_lru_;
	Cache<false> random_cache_;
	Cache<true> lru_cache_;
};

class CacheSim
{
public:
	// stack trace will be huge and expensive to move twice
	CacheSim(CacheConf cache_conf, const StackTrace &stack_trace)
		: cache_conf_(std::move(cache_conf)), stack_trace_(std::move(stack_trace)){};
	CacheSim(CacheConf cache_conf, StackTrace &&stack_trace) noexcept
		: cache_conf_(std::move(cache_conf)), stack_trace_(std::move(stack_trace)){};

	/**
	 * @brief Run the simulation for the current stack trace
	 * and cache config. Stores the results in results_.
	 **/
	void RunSimulation();

	void set_cache_config(CacheConf cache_conf)
	{
		cache_conf_ = cache_conf;
	};

	CacheConf get_cache_config() const
	{
		return cache_conf_;
	};

	void set_stack_trace(const StackTrace &stack_trace)
	{
		stack_trace_ = std::move(stack_trace);
	};

	void set_stack_trace(StackTrace &&stack_trace) noexcept
	{
		stack_trace_ = std::move(stack_trace);
	};

	Results results();

private:
	CacheConf cache_conf_;
	StackTrace stack_trace_;
	Results results_;
};
