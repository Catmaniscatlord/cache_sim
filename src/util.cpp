/**
 * filename: util.cpp
 *
 * description: utility functions
 *
 * authors: Chamberlain, David
 *
 **/

#include "util.hpp"

#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <string>

#include "base_structs.hpp"

namespace Util
{

std::optional<CacheConf> ReadCacheConfFile(const std::string &s)
{
	CacheConf conf{};
	std::ifstream file(s, std::ios_base::in);
	if (!file)
		return {};

	unsigned int tmp;
	file >> tmp;
	conf.line_size_ = static_cast<uint_fast8_t>(tmp);
	file >> tmp;
	conf.associativity_ = static_cast<uint_fast8_t>(tmp);
	file >> tmp;
	conf.cache_size_ = tmp;
	// Kb to bytes
	conf.cache_size_ *= 1024;
	file >> tmp;
	switch (tmp)
	{
		case 0:
			conf.replacement_policy_ = ReplacementPolicy::RAND;
			break;
		case 1:
			conf.replacement_policy_ = ReplacementPolicy::FIFO;
			break;
		default:
			__builtin_unreachable();
	}
	file >> tmp;
	conf.miss_penalty_ = static_cast<uint_fast8_t>(tmp);
	file >> tmp;
	conf.write_allocate_ = static_cast<uint_fast8_t>(tmp);

	return conf;
}

std::optional<StackTrace> ReadStackTraceFile(const std::string &s)
{
	StackTrace st;
	std::ifstream file(s, std::ios_base::in);
	if (!file)
		return {};

	char is_read;
	std::string address;
	unsigned int last_memory_access_count;
	MemoryAccess ma{};
	while (file >> is_read >> address >> last_memory_access_count)
	{
		if (is_read == 'l')
			ma.is_read = true;
		else
			ma.is_read = false;

		ma.address =
			static_cast<address_t>(std::stoul(address.substr(2), 0, 16));
		ma.last_memory_access_count =
			static_cast<uint_fast8_t>(last_memory_access_count);

		st.push_back(std::move(ma));
	}

	return st;
}
}  // namespace Util
