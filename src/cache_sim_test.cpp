/**
 * filename: cache_sim_test.cpp
 *
 * description: Runs test on the cache simulator
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

#include <gtest/gtest.h>

TEST(CacheSimTest, directMapped)
{
	CacheConf cc{2, 1, 1, 8, 1, 1};
	// 2 bytes for a line
	// 2 lines per index
	// 8 bytes total
	// 4 bytes per index
	// 4 indecies
	// 1 bit per index
	// 1 bit offset
	// FIFO
	CacheWrapper cache{cc};
	ASSERT_FALSE(cache.AccessMemory(0b1011, true));	 // first index tag 0001
													 // Index state : 0b1010
	ASSERT_TRUE(cache.AccessMemory(0b1010, true));	 // first index tag 0001
													 // Index state : 0b1010
	ASSERT_FALSE(cache.AccessMemory(0b0011, true));	 // first index tag 0001
													 // Index state : 0b0011
	ASSERT_FALSE(cache.AccessMemory(0b1010, true));	 // first index tag 0001
}

TEST(CacheSimTest, associativity)
{
	CacheConf cc{2, 2, 1, 8, 1, 1};
	// 2 bytes for a line
	// 2 lines per index
	// 8 bytes total
	// 4 bytes per index
	// 2 indecies
	// 1 bit per index
	// 1 bit offset
	// FIFO
	CacheWrapper cache{cc};
	ASSERT_FALSE(cache.AccessMemory(0b111, true));	// first index tag 0001
													// Index state : 0b111
	ASSERT_FALSE(cache.AccessMemory(0b011, true));	// first index tag 0000
													// Index state : 0b111 0b011
	ASSERT_FALSE(cache.AccessMemory(0b1011, true));	 // first index tag 0010
	// Index state : 0b1011 0b011
	ASSERT_FALSE(cache.AccessMemory(0b111, true));	// first index tag 0001
	// Index state : 0b1011 0b111
	ASSERT_TRUE(cache.AccessMemory(0b1011, true));	// first index tag 0010
}

TEST(CacheSimTest, writePolicy)
{
	CacheConf cc{2, 2, 1, 8, 0, 1};
	// 2 bytes for a line
	// 2 lines per index
	// 8 bytes total
	// 4 bytes per index
	// 2 indecies
	// 1 bit per index
	// 1 bit offset
	// FIFO
	CacheWrapper cache{cc};
	ASSERT_FALSE(cache.AccessMemory(0b111, false));	 // first index tag 0001
	ASSERT_FALSE(cache.AccessMemory(0b111, false));	 // first index tag 0001
	ASSERT_FALSE(cache.AccessMemory(0b111, false));	 // first index tag 0001
	ASSERT_FALSE(cache.AccessMemory(0b111, false));	 // first index tag 0001
	ASSERT_FALSE(cache.AccessMemory(0b111, false));	 // first index tag 0001
}
