/**
 * filename: cache.hpp
 *
 * description: header file for a cache
 *
 * authors: Chamberlain, David
 **/

#pragma once

#include <cstdint>
#include <unordered_set>

#include "base_structs.hpp"
#include "cache_block.hpp"

/**
 * @brief one associative container of the cache
 * @description has as many elements as the level of associativity.
 * @param ReplaceContainer Stores the cache blocks, and is used for the
 *replacement policy
 **/
template <CacheBlockContainer T>
struct CacheIndex
{
	using SearchMap = std::unordered_set<typename T::iterator,
										 CacheBlockHash<T>,
										 CacheBlockCompare<T>>;

	const uint_fast8_t associativity_;

	T list;

	SearchMap map;

	CacheIndex(uint_fast8_t associativity,
			   const CacheBlockCompare<T> &compare,
			   const CacheBlockHash<T> &hash)
		: associativity_(associativity),
		  list(associativity),
		  map(static_cast<size_t>(associativity), hash, compare)
	{
		map.reserve(associativity_);
	};

	// clear the maps and lists, effectively flushes the cache
	void clear()
	{
		map.clear();
		list.clear();
	};
};

/**
 * @brief Vitrual base class for caches
 * @description This is a base class for our spefic cache implementations. This
 *allows us to construct different cache types with our cache factory, and treat
 *them as the same class
 **/
struct CacheBase
{
	const address_t cache_size_;
	const uint_fast8_t associativity_;
	const uint_fast8_t line_size_;
	const address_t num_indicies_;
	const uint_fast8_t offset_size_;
	const uint_fast8_t index_size_;
	const uint_fast8_t tag_size_;
	const uint_fast8_t tag_shift_;
	/**
	 * false : no-write allocate, write-through
	 * true : write-allocate, write-back
	 */
	const bool is_write_allocate_;
	const ReplacementPolicy replacement_policy_;

	CacheBase(CacheConf cc)
		: cache_size_(cc.cache_size_),
		  associativity_(cc.associativity_),
		  line_size_(cc.line_size_),
		  // if the associativity is 0, then the cache is fully associative,
		  // there is only 1 index
		  num_indicies_(
			  cc.associativity_
				  ? cc.cache_size_ / (cc.associativity_ * cc.line_size_)
				  : 1),
		  offset_size_(
			  static_cast<uint_fast8_t>(std::bit_width(cc.line_size_) - 1)),
		  index_size_(
			  static_cast<uint_fast8_t>(std::bit_width(num_indicies_) - 1)),
		  tag_size_(static_cast<uint_fast8_t>(
			  8 * sizeof(address_t) - offset_size_ - index_size_)),
		  tag_shift_(offset_size_ + index_size_),
		  is_write_allocate_(cc.write_allocate_),
		  replacement_policy_(cc.replacement_policy_){};

	virtual ~CacheBase() = default;

	/**
	 * @brief returns true on hit, false on miss
	 * @description Two different instantiations for when
	 * the cache uses random vs fifo for replacement
	 **/
	virtual bool AccessMemory(const address_t &address, const bool &read) = 0;

	// flush the cache by clearing each cache index
	virtual void ClearCache() = 0;

	inline auto get_index(address_t address) const
	{
		return (address >> offset_size_) & ((1 << index_size_) - 1);
	};
};

/**
 * @brief A cache that we can "store" memory in
 * @description handles some base operations for all caches
 **/
template <CacheBlockContainer T>
class Cache : public CacheBase
{
private:
	const CacheBlockCompare<T> comparer;
	const CacheBlockHash<T> hasher;

protected:
	std::vector<CacheIndex<T>> cache_;

public:
	Cache(CacheConf cc)
		: CacheBase(cc),
		  comparer(tag_shift_),
		  hasher(tag_shift_),
		  cache_(num_indicies_, {associativity_, comparer, hasher}){};

	virtual ~Cache() = default;

	/**
	 * @brief returns true on hit, false on miss
	 * @description Two different instantiations for when the cache uses random
	 *vs fifo for replacement
	 **/
	virtual bool
	AccessMemory(const address_t &address, const bool &read) override = 0;

	// flush the cache by clearing each cache index
	void ClearCache() override
	{
		for (auto &ci : cache_)
			ci.clear();
	};
};
