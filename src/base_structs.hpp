/**
 * filename: base_structs.hpp
 *
 * description: basic structures that are used throught the program
 *
 * authors: Chamberlain, David
 **/

#pragma once

#include <cstdint>
#include <ostream>
#include <vector>

using address_t = uint32_t;

struct MemoryAccess
{
	address_t address;

	// max value of 65536, largest that can be held while keeping the struct at
	// 8 bytes
	uint16_t last_memory_access_count;

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

enum ReplacementPolicy
{
	RAND,
	FIFO
};

/**
 * @param uint_fast8_t line_size
 * @param uint_fast8_t associativity
 * @param uint_fast8_t miss_penalty
 * @param address_t cache_size
 * @param bool write_allocate
 * @param ReplacementPolicy replacement_policy
 */

struct CacheConf
{
	uint_fast8_t line_size_;
	uint_fast8_t associativity_;
	bool write_allocate_;
	/**
	 * false : no-write allocate, write-through
	 * true : write-allocate, write-back
	 */
	uint_fast8_t miss_penalty_;
	address_t cache_size_;
	/**
	 * false : random Replacement
	 * true : FIFO replacement
	 */
	ReplacementPolicy replacement_policy_;

	CacheConf() = default;

	constexpr CacheConf(
		uint_fast8_t line_size,
		uint_fast8_t associativity,
		address_t cache_size,
		ReplacementPolicy replacement_policy,
		uint_fast8_t miss_penalty,
		bool write_allocate)
		: line_size_(line_size),
		  associativity_(associativity),
		  write_allocate_(write_allocate),
		  miss_penalty_(miss_penalty),
		  cache_size_(cache_size),
		  replacement_policy_(replacement_policy){};
};

struct Results
{
	double total_hit_rate;
	double read_hit_rate;
	double write_hit_rate;
	uint64_t run_time;
	double average_memory_access_time;
};
