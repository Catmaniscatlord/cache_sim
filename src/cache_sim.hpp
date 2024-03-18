/**
 * filename: cache_sim.hpp
 *
 * description: header file for our cache simulator
 *
 * authors: Chamberlain, David
 **/

#pragma once

#include <boost/circular_buffer.hpp>
#include <boost/concept_check.hpp>
#include <utility>

#include "base_structs.hpp"
#include "cache_factory.hpp"

/**
 * @brief cache simulator
 * @ make sure to clear the cache after s simulation for accurate results
 **/
class CacheSimulator
{
private:
	std::unique_ptr<CacheBase> cache_;
	CacheConf cache_conf_;
	// internal storage for the stack trace if needed

public:
	CacheSimulator(CacheConf cache_conf)
		: cache_{CacheFactory::CreateCache(cache_conf)}, cache_conf_{cache_conf}
	{}

	template <typename Arg>
	CacheSimulator(CacheConf cache_conf, Arg&& stack_trace)
		: cache_(CacheFactory::CreateCache(cache_conf)), cache_conf_(cache_conf)
	{
		set_stack_trace(std::forward<Arg>(stack_trace));
	}

	/**
	 * @brief Run the simulation for the current stack trace and cache config.
	 *Stores the results in results_.
	 **/
	Results SimulateTrace(const StackTrace& st);

	CacheConf get_cache_config() const
	{
		return cache_conf_;
	};

	/**
	 * @brief clears the cache
	 **/
	void ClearCache()
	{
		cache_->ClearCache();
	}
};
