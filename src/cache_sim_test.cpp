/**
 * filename: cache_sim_test.cpp
 *
 * description: Runs test on the cache simulator
 *
 * authors: Chamberlain, David
 *
 **/

#include <gtest/gtest.h>

#include <memory>

#include "base_structs.hpp"
#include "cache.hpp"
#include "cache_block.hpp"
#include "cache_factory.hpp"

TEST(CacheSimTest, cacheConfig)
{
	constexpr CacheConf cc{32, 2, 128 * 1024, ReplacementPolicy::FIFO, 70, 0};
	std::unique_ptr<CacheBase> cache{CacheFactory::CreateCache(cc)};

	ASSERT_EQ(cache->cache_size_, 128 * 1024);
	ASSERT_EQ(cache->associativity_, 2);
	ASSERT_EQ(cache->index_size_, 11);
	ASSERT_EQ(cache->offset_size_, 5);
	ASSERT_EQ(cache->num_indicies_, (128 * 1024) / (32 * 2));
	ASSERT_EQ(cache->is_write_allocate_, 0);
	ASSERT_EQ(cache->tag_size_, 16);
}

TEST(CacheSimTest, cacheIndex)
{
	constexpr CacheConf cc{32, 2, 128 * 1024, ReplacementPolicy::FIFO, 70, 0};
	std::unique_ptr<CacheBase> cache{CacheFactory::CreateCache(cc)};
	// offset 5 bits
	// index 11 bits

	// index 10000101000
	cache_block_t cb1{0b00000000000000011000010100010000, 1};

	ASSERT_EQ(cache->get_index(cb1.block_address), 0b10000101000);
}

TEST(CacheSimTest, cacheSet)
{
	constexpr CacheConf cc{32, 2, 128 * 1024, ReplacementPolicy::FIFO, 70, 0};
	std::unique_ptr<CacheBase> cache{CacheFactory::CreateCache(cc)};
	const uint_fast8_t tag_shift{
		static_cast<uint_fast8_t>(sizeof(address_t) * 8 - cache->tag_size_)};

	ASSERT_EQ(tag_shift, 16);

	CacheBlockHash<std::vector<cache_block_t>> hash{tag_shift};
	CacheBlockCompare<std::vector<cache_block_t>> compare{tag_shift};

	cache_block_t cb1{0b00000000000000010000000000000000, 1};
	cache_block_t cb2{0b00000000000000011111111111111111, 1};
	cache_block_t cb3{0b00000000000000110000000000000000, 1};
	cache_block_t cb4{0b00000000000000111111111111111111, 1};

	std::vector<cache_block_t> cbl;
	cbl.push_back(cb1);
	cbl.push_back(cb2);
	cbl.push_back(cb3);
	cbl.push_back(cb4);

	ASSERT_EQ(hash(cb1), hash(cb2));
	ASSERT_NE(hash(cb1), hash(cb3));
	ASSERT_EQ(hash(cb3), hash(cb4));
	ASSERT_NE(hash(cb2), hash(cb4));

	ASSERT_EQ(hash(std::next(cbl.begin(), 0)), hash(std::next(cbl.begin(), 1)));
	ASSERT_NE(hash(std::next(cbl.begin(), 0)), hash(std::next(cbl.begin(), 2)));
	ASSERT_EQ(hash(std::next(cbl.begin(), 2)), hash(std::next(cbl.begin(), 3)));
	ASSERT_NE(hash(std::next(cbl.begin(), 1)), hash(std::next(cbl.begin(), 3)));

	ASSERT_EQ(hash(std::next(cbl.begin(), 0)), hash(cb1));
	ASSERT_EQ(hash(std::next(cbl.begin(), 1)), hash(cb2));
	ASSERT_EQ(hash(std::next(cbl.begin(), 2)), hash(cb3));
	ASSERT_EQ(hash(std::next(cbl.begin(), 3)), hash(cb4));

	ASSERT_TRUE(compare(std::next(cbl.begin(), 0), cb1));
	ASSERT_TRUE(compare(std::next(cbl.begin(), 1), cb2));
	ASSERT_TRUE(compare(std::next(cbl.begin(), 2), cb3));
	ASSERT_TRUE(compare(std::next(cbl.begin(), 3), cb4));
}

TEST(CacheSimTest, directMapped)
{
	CacheConf cc{2, 1, 8, ReplacementPolicy::RAND, 1, 1};
	// 2 bytes for a line
	// 1 lines per index
	// 8 bytes total
	// 2 bytes per index
	// 4 indecies
	// 2 bit per index
	// 1 bit offset
	// FIFO
	std::unique_ptr<CacheBase> cache{CacheFactory::CreateCache(cc)};
	cache->ClearCache();
	ASSERT_FALSE(cache->AccessMemory(0b1011, true));  // first index tag 01
													  // Index state : 0b1
	ASSERT_TRUE(cache->AccessMemory(0b1010, true));	  // first index tag 01
													  // Index state : 0b1
	EXPECT_FALSE(cache->AccessMemory(0b0011, true));  // first index tag 01
													  // Index state : 0b0
	ASSERT_FALSE(cache->AccessMemory(0b1010, true));  // first index tag 01
}

TEST(CacheSimTest, associativity)
{
	CacheConf cc{2, 2, 8, ReplacementPolicy::FIFO, 1, 1};
	// 2 bytes for a line
	// 2 lines per index
	// 8 bytes total
	// 4 bytes per index
	// 2 indecies
	// 1 bit per index
	// 1 bit offset
	// FIFO
	std::unique_ptr<CacheBase> cache{CacheFactory::CreateCache(cc)};
	ASSERT_FALSE(cache->AccessMemory(0b111, true));	 // first index tag 0001
													 // Index state : 0b111
	ASSERT_FALSE(
		cache->AccessMemory(0b011, true));	// first index tag 0000
											// Index state : 0b111 0b011
	ASSERT_FALSE(cache->AccessMemory(0b1011, true));  // first index tag 0010
	// Index state : 0b1011 0b011
	ASSERT_FALSE(cache->AccessMemory(0b111, true));	 // first index tag 0001
	// Index state : 0b1011 0b111
	ASSERT_TRUE(cache->AccessMemory(0b1011, true));	 // first index tag 0010
}

TEST(CacheSimTest, writePolicy)
{
	CacheConf cc{2, 2, 8, ReplacementPolicy::FIFO, 1, 0};
	// 2 bytes for a line
	// 2 lines per index
	// 8 bytes total
	// 4 bytes per index
	// 2 indecies
	// 1 bit per index
	// 1 bit offset
	// FIFO
	std::unique_ptr<CacheBase> cache{CacheFactory::CreateCache(cc)};
	ASSERT_FALSE(cache->AccessMemory(0b111, false));  // first index tag 0001
	ASSERT_FALSE(cache->AccessMemory(0b111, false));  // first index tag 0001
	ASSERT_FALSE(cache->AccessMemory(0b111, false));  // first index tag 0001
	ASSERT_FALSE(cache->AccessMemory(0b111, false));  // first index tag 0001
	ASSERT_FALSE(cache->AccessMemory(0b111, false));  // first index tag 0001
}
