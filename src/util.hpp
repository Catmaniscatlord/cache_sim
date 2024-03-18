/**
 * filename: util.hpp
 *
 * description: header file for utility functions
 *
 * authors: Chamberlain, David
 *
 **/

#pragma once

#include <chrono>
#include <iostream>
#include <optional>

#include "cache_sim.hpp"

namespace Util
{
std::optional<CacheConf> ReadCacheConfFile(const std::string &s);
std::optional<StackTrace> ReadStackTraceFile(const std::string &s);

struct Timer
{
	Timer(std::string s, bool print = false) : name{s}
	{
		start(print);
	};

	const std::string name;
	decltype(std::chrono::high_resolution_clock::now()) start_;

	decltype(std::chrono::high_resolution_clock::now()) stop_;

	void start(bool print = false)
	{
		start_ = std::chrono::high_resolution_clock::now();
		if (print)
			std::cout << "Timer : " << name << " started" << std::endl;
	}

	void stop(bool print = false)
	{
		stop_ = std::chrono::high_resolution_clock::now();
		if (print)
			std::cout << "Timer : " << name << " stopped" << std::endl;
	}

	void print()
	{
		std::cout << "timer : " << name << "   "
				  << duration_cast<std::chrono::milliseconds>(stop_ - start_)
				  << std::endl;
	}
};

}  // namespace Util
