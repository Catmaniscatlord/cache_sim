/**
 * filename: cache_sim.hpp
 *
 * description: header file for our cache simulator
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
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <cstdint>
#include <deque>
#include <iostream>
#include <list>
#include <ostream>
#include <random>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

using address_t = uint32_t;

/**
 * @param uint_fast8_t line_size
 * @param uint_fast8_t associativity
 * @param uint_fast8_t miss_penalty
 * @param address_t cache_size
 * @param bool write_allocate
 * @param bool replacement_policy
 */

struct CacheConf
{
	uint_fast8_t line_size;
	uint_fast8_t associativity;
	uint_fast8_t miss_penalty;
	address_t cache_size;
	/**
	 * false : no-write allocate, write-through
	 * true : write-allocate, write-back
	 */
	bool write_allocate;
	/**
	 * false : random Replacement
	 * true : FIFO replacement
	 */
	bool is_fifo;
};

struct Results
{
	double total_hit_rate;
	double read_hit_rate;
	double write_hit_rate;
	uint64_t run_time;
	double average_memory_access_time;
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

	friend std::ostream &operator<<(std::ostream &os, const MemoryAccess &ma)
	{
		os << (ma.is_read ? "l" : "s") << " "
		   << "0x" << std::hex << ma.address << " "
		   << static_cast<unsigned int>(ma.last_memory_access_count);
		return os;
	};
};

typedef std::vector<MemoryAccess> StackTrace;

struct cache_block_t
{
	address_t block_address_;
	// the dirty bit and its functionality is useless unless he gives some clock
	// times for write-through and write-back
	bool dirty_;

	cache_block_t() = default;

	cache_block_t(address_t address, bool dirty)
		: block_address_(address), dirty_(dirty){};
};

/**
 * @brief one associative container of the cache
 * @description Used to contain all the blocks within a single index. There are
 *as many of these in the cache as the level of associativity
 * @ is_fifo = true
 * @ map is a hashmap maping blocks to a location in the fifo list
 * @ list is a FIFO list of the blocks
 * @ is_fifo = false
 * @ map is a set containing the blocks for constant time look up
 * @ list is an array of the blocks, for constant time access
 **/
template <bool is_fifo>
struct CacheIndex
{
	// These could be made faster if they pointed to where the cache blocks were
	// in the block map. Either way its O(1)
	using ReplaceList = std::conditional_t<is_fifo,
										   std::list<cache_block_t>,
										   std::deque<cache_block_t>>;

	// This allows us to compare the stored list iterator, with its underlying
	// cache block
	struct CompareBlock
	{
		using is_transparent = void;

		const uint_fast8_t tag_shift;

		bool
		operator()(const typename ReplaceList::iterator &lhs,
				   const typename ReplaceList::iterator &rhs) const noexcept
		{
			return (lhs->block_address_ >> tag_shift) ==
				   (rhs->block_address_ >> tag_shift);
		}

		bool
		operator()(const cache_block_t &lhs,
				   const typename ReplaceList::iterator &rhs) const noexcept
		{
			return (lhs.block_address_ >> tag_shift) ==
				   (rhs->block_address_ >> tag_shift);
		}

		bool operator()(const typename ReplaceList::iterator &lhs,
						const cache_block_t &rhs) const noexcept
		{
			return (rhs.block_address_ >> tag_shift) ==
				   (lhs->block_address_ >> tag_shift);
		}
	};

	// This allows us to search if a block is in the map by passing just the
	// block, instead of storing it in a list/vector first
	struct HashBlock
	{
		using is_transparent = void;

		const uint_fast8_t tag_shift;

		std::size_t operator()(const cache_block_t &cb) const noexcept
		{
			return (cb.block_address_ >> tag_shift);
		}

		std::size_t
		operator()(const typename ReplaceList::iterator &cbp) const noexcept
		{
			return (cbp->block_address_ >> tag_shift);
		}
	};

	// If fifo, the list is an fifo list of the blocks. If random, the list is a
	// vector that can be randomly accessed that points to a location in the
	// cache
	ReplaceList list;

	using BlockMap = std::
		unordered_set<typename ReplaceList::iterator, HashBlock, CompareBlock>;

	BlockMap map;

	uint_fast8_t associativity_;

	CacheIndex(uint_fast8_t associativity, uint_fast8_t tag_size)
		: associativity_(associativity),
		  map(static_cast<size_t>(associativity),
			  HashBlock{
				  static_cast<uint_fast8_t>(8 * sizeof(address_t) - tag_size)},
			  CompareBlock{static_cast<uint_fast8_t>(
				  8 * sizeof(address_t) - tag_size)}){};

	// clear the maps and lists, effectively flushes the cache
	void clear()
	{
		map.clear();
		list.clear();
	}
};

/**
 * @brief A cache that we can "store" memory in
 * @description this is the cache object that we can simulate memory access
 *with. If the cache uses fifo, the data for the cache has a different structure
 *than for random access. Hence the wrapper.
 **/
template <bool is_fifo>
class Cache
{
public:
	Cache()
		: cache_size_(),
		  associativity_(),
		  line_size_(),
		  is_write_allocate_(),
		  num_indicies_(),
		  offset_size_(),
		  index_size_(),
		  tag_size_(){};

	Cache(CacheConf cc)
		: cache_size_(cc.cache_size),
		  associativity_(cc.associativity),
		  line_size_(cc.line_size),
		  is_write_allocate_(cc.write_allocate),
		  // if the associativity is 0, then the cache is fully associative,
		  // there is only 1 index
		  num_indicies_(cc.associativity
							? cc.cache_size / (cc.associativity * cc.line_size)
							: 1),
		  offset_size_(
			  static_cast<uint_fast8_t>(std::bit_width(cc.line_size) - 1)),
		  index_size_(
			  static_cast<uint_fast8_t>(std::bit_width(num_indicies_) - 1)),
		  tag_size_(static_cast<uint_fast8_t>(
			  8 * sizeof(address_t) - offset_size_ - index_size_))
	{
		cache_.reserve(num_indicies_);
		for (int i = 0; i < num_indicies_; i++)
			cache_.emplace_back(CacheIndex<is_fifo>(associativity_, tag_size_));
	};

	/**
	 * @brief returns true on hit, false on miss
	 * @description Two different instantiations for when
	 * the cache uses random vs fifo for replacement
	 **/
	bool AccessMemory(const address_t &address, const bool &read)
	requires is_fifo;
	bool AccessMemory(const address_t &address, const bool &read)
	requires (not is_fifo);

	// flush the cache by clearing each cache index
	void ClearCache()
	{
		for (auto &ci : cache_)
			ci.clear();
	}

private:
	// used for random removal in a random replacement_policy cache
	static std::mt19937 kGen;

	const address_t cache_size_;
	const uint_fast8_t associativity_;
	const uint_fast8_t line_size_;
	const address_t num_indicies_;
	const uint_fast8_t offset_size_;
	const uint_fast8_t index_size_;
	const uint_fast8_t tag_size_;
	/**
	 * false : no-write allocate, write-through
	 * true : write-allocate, write-back
	 */
	const bool is_write_allocate_;

	std::vector<CacheIndex<is_fifo>> cache_;
};

/**
 * @brief wraps the cache
 * @description Wraps the two different types of cache so
 * we can access the cache as if it was a single type.
 **/
class CacheWrapper
{
public:
	static CacheWrapper getWrapper(const CacheConf &cc)
	{
		if (cc.is_fifo)
			return {Cache<true>{cc}};
		else
			return {Cache<false>{cc}};
	}

	template <bool is_fifo>
	requires is_fifo
	CacheWrapper(const Cache<is_fifo> &cache)
		: is_fifo_(is_fifo), fifo_cache_(cache){};

	template <bool is_fifo>
	requires (not is_fifo)
	CacheWrapper(const Cache<is_fifo> &cache)
		: is_fifo_(is_fifo), random_cache_(cache){};

	/**
	 * @brief returns true on hit, false on miss
	 **/
	auto AccessMemory(const address_t &address, const bool &read)
	{
		if (is_fifo_)
			return fifo_cache_.AccessMemory(address, read);
		else
			return random_cache_.AccessMemory(address, read);
	}

	/**
	 * @brief clears the cache
	 **/
	auto ClearCache()
	{
		if (is_fifo_)
			return fifo_cache_.ClearCache();
		else
			return random_cache_.ClearCache();
	}

private:
	bool is_fifo_;
	Cache<false> random_cache_;
	Cache<true> fifo_cache_;
};

/**
 * @brief cache simulator
 * @ make sure to clear the cache after s simulation for accurate results
 **/
class CacheSimulator
{
public:
	CacheSimulator(CacheConf cache_conf)
		: cache_wrapper_(CacheWrapper::getWrapper(cache_conf)),
		  cache_conf_(cache_conf),
		  stack_trace_ref_(&stack_trace_)
	{}

	template <typename Arg>
	CacheSimulator(CacheConf cache_conf, Arg &&stack_trace)
		: cache_wrapper_(CacheWrapper::getWrapper(cache_conf)),
		  cache_conf_(cache_conf)
	{
		set_stack_trace(std::forward<Arg>(stack_trace));
	};

	/**
	 * @brief Run the simulation for the current stack trace and cache config.
	 *Stores the results in results_.
	 **/
	void RunSimulation();

	CacheConf get_cache_config() const
	{
		return cache_conf_;
	};

	// This allows us to store references to objects
	// as well as store them if desired
	template <typename T>
	void set_stack_trace(T &&stack_trace)
	{
		if constexpr (std::is_pointer_v<T>)
		{
			stack_trace_ref_ = stack_trace;
			stack_trace_.clear();
		}
		else if constexpr (std::is_lvalue_reference_v<T>)
		{
			stack_trace_ref_ = &stack_trace;
			stack_trace_.clear();
		}
		else
		{
			stack_trace_ = stack_trace;
			stack_trace_ref_ = &stack_trace_;
		}
	};

	/**
	 * @brief clears the cache
	 **/
	void ClearCache()
	{
		cache_wrapper_.ClearCache();
	}

	Results results()
	{
		return results_;
	};

private:
	CacheWrapper cache_wrapper_;
	CacheConf cache_conf_;
	// change this to a shared pointer for improved efficiency
	StackTrace stack_trace_;
	StackTrace *stack_trace_ref_;
	Results results_;
};
