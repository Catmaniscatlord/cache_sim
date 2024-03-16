/**
 * filename: cache_block.hpp
 *
 * description: Definitions surrounding cache blocks, their containers, and
 *their comparisons
 *
 * authors: Chamberlain, David
 **/
#pragma once

#include <cstdint>

#include "base_structs.hpp"

struct cache_block_t
{
	address_t block_address;
	bool dirty;
};

template <typename T>
concept CacheBlockContainer = requires(T t, size_t x) {
	// contains cache blocks
	std::is_same_v<typename T::iterator::value_type, cache_block_t>;
	// we can clear it, so we can flush the cache
	t.clear();
	// require that the container can be initalized with x elements
	t = T(x);
	t.size() == x;
};

template <CacheBlockContainer T>
struct CacheBlockCompare
{
	// this allows for unordered set to use any of these functions for
	// comparison
	using is_transparent = void;

	const uint_fast8_t tag_shift;

	CacheBlockCompare(uint_fast8_t tag_shift_) : tag_shift(tag_shift_){};

	bool operator()(const typename T::iterator &lhs,
					const typename T::iterator &rhs) const noexcept
	{
		return (lhs->block_address >> tag_shift) ==
			   (rhs->block_address >> tag_shift);
	};

	bool operator()(const cache_block_t &lhs,
					const typename T::iterator &rhs) const noexcept
	{
		return (lhs.block_address >> tag_shift) ==
			   (rhs->block_address >> tag_shift);
	};

	bool operator()(const typename T::iterator &lhs,
					const cache_block_t &rhs) const noexcept
	{
		return (lhs->block_address >> tag_shift) ==
			   (rhs.block_address >> tag_shift);
	};
};

template <CacheBlockContainer T>
struct CacheBlockHash
{
	// this allows for unordered set to use any of these functions for
	// comparison
	using is_transparent = void;

	const uint_fast8_t tag_shift;

	CacheBlockHash(uint_fast8_t tag_shift_) : tag_shift(tag_shift_){};

	std::size_t operator()(const cache_block_t &cb) const noexcept
	{
		return (cb.block_address >> tag_shift);
	};

	std::size_t operator()(const typename T::iterator &cbp) const noexcept
	{
		return (cbp->block_address >> tag_shift);
	};
};
