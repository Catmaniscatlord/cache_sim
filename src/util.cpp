/**
 * filename: util.cpp
 *
 * description: utility functions
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

#include "util.hpp"

#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <string>

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
	conf.line_size = static_cast<uint_fast8_t>(tmp);
	file >> tmp;
	conf.associativity = static_cast<uint_fast8_t>(tmp);
	file >> tmp;
	conf.cache_size = tmp;
	// Kb to bytes
	conf.cache_size *= 1024;
	file >> tmp;
	conf.replacement_policy = tmp;
	file >> tmp;
	conf.miss_penalty = static_cast<uint_fast8_t>(tmp);
	file >> tmp;
	conf.write_allocate = static_cast<uint_fast8_t>(tmp);

	return conf;
}

std::optional<CacheConf> ReadCacheConfFile(std::string &&s)
{
	CacheConf conf{};
	std::ifstream file(s, std::ios_base::in);
	if (!file)
		return {};

	unsigned int tmp;
	file >> tmp;
	conf.line_size = static_cast<uint_fast8_t>(tmp);
	file >> tmp;
	conf.associativity = static_cast<uint_fast8_t>(tmp);
	file >> tmp;
	conf.cache_size = tmp;
	// Kb to bytes
	conf.cache_size *= 1024;
	file >> tmp;
	conf.replacement_policy = tmp;
	file >> tmp;
	conf.miss_penalty = static_cast<uint_fast8_t>(tmp);
	file >> tmp;
	conf.write_allocate = static_cast<uint_fast8_t>(tmp);

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

std::optional<StackTrace> ReadStackTraceFile(std::string &&s)
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
