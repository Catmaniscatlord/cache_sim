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

#include <bit>
#include <cstdint>
#include <list>
#include <random>
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
		return (cb.block_address >> ((8 * sizeof(address_t)) - cb.tag_size));
	}
};

/**
 * @breif Used to contain all the blocks within a single index.
 * There are as many of these in the cache as the level of associativity
 * @description
 * @ is_lru = true 
 * @ map is a hashmap maping blocks to a location in the LRU list
 * @ list is a LRU list of the blocks
 * @ is_lru = false 
 * @ map is a set containing the blocks for constant time look up
 * @ list is an array of the blocks, for constant time access
 **/
template <bool is_lru>
struct CacheIndex
{
	// These could be made faster if they pointed to where the cache
	// blocks were in the block map. Either way its O(1)
	using ReplaceList = std::conditional_t<is_lru,
											std::list<cache_block_t>,
											std::vector<cache_block_t>>;

	// If LRU, the list is an LRU list of the blocks. 
	// If random, the list is a vector that can be randomly accessed
	// that points to a location in the cache
	ReplaceList list;

	using BlockMap = std::conditional_t<is_lru,
										std::unordered_map<cache_block_t, decltype(list.begin())>,
										std::unordered_set<cache_block_t>>;

	BlockMap map;

	CacheIndex(address_t cache_set_size)
	{
		map.reserve(cache_set_size);

		if constexpr(not is_lru)
			list.reserve(cache_set_size);
	};
};

/**
 * @breif this is the cache object that we can simulate memory access with.
 * If the cache uses LRU, the data for the cache has a different structure than
 * for random access. Hence the wrapper.
 **/
template <bool is_lru>
class Cache
{
public:
	// we have a defined constructor so we must
	// explicitly set a default for move semantics
	Cache() = default;

	Cache(CacheConf cc)
	{
		cache_size_ = cc.cache_size;
		associativity_ = cc.associativity;
		block_size_ = cc.block_size;
		is_write_allocate_ = cc.write_allocate;
		cache_set_size_ = (cc.cache_size / cc.associativity) / cc.block_size,
		offset_size_ = std::bit_width(cc.block_size);
		index_size_ = std::bit_width(cc.associativity);

		tag_size_ = 8 * sizeof(address_t) - offset_size_ - index_size_;

		cache_.reserve(associativity_);
		CacheIndex<is_lru>{cache_set_size_};
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
	static std::mt19937 kGen;	

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
 * @brief Wraps the two different types of the Chache so
 * we can access the cache as if it was a single type.
 **/
class CacheWrapper
{
public:
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
	void set_cache(Cache<is_lru> &&cache) noexcept
	{
		is_lru_ = is_lru;
		lru_cache_ = std::move(cache);
	};

	template <bool is_lru>
		requires(not is_lru)
	void set_cache(Cache<is_lru> &&cache) noexcept
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
	CacheSim () = default;

	// stack trace will be huge and expensive to move twice
	CacheSim(CacheConf cache_conf, const StackTrace &stack_trace)
		: cache_conf_(std::move(cache_conf)), stack_trace_(std::move(stack_trace)){};
	CacheSim(CacheConf cache_conf, StackTrace &&stack_trace) noexcept
		: cache_conf_(std::move(cache_conf)), stack_trace_(std::move(stack_trace)){};

	/**
	 * @brief Run the simulation for the current stack trace
	 * and cache config. Stores the results in results_.
	 *
	 **/
	void RunSimulation();

	void set_cache_config(CacheConf cache_conf)
	{
		cache_conf_ = cache_conf;

		if (cache_conf_.miss_penalty)
			cache_wrapper_.set_cache(Cache<true>{cache_conf_});
		else
			cache_wrapper_.set_cache(Cache<false>{cache_conf_});
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
	CacheWrapper cache_wrapper_;
	CacheConf cache_conf_;
	StackTrace stack_trace_;
	Results results_;
};
