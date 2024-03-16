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
#include <type_traits>
#include <utility>

#include "cache_factory.hpp"

/**
 * @brief cache simulator
 * @ make sure to clear the cache after s simulation for accurate results
 **/
class CacheSimulator
{
public:
	CacheSimulator(CacheConf cache_conf)
		: cache_(CacheFactory::CreateCache(cache_conf)),
		  cache_conf_(cache_conf),
		  stack_trace_ref_(&stack_trace_)
	{}

	template <typename Arg>
	CacheSimulator(CacheConf cache_conf, Arg &&stack_trace)
		: cache_(CacheFactory::CreateCache(cache_conf)), cache_conf_(cache_conf)
	{
		set_stack_trace(std::forward<Arg>(stack_trace));
	}

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
	}

	/**
	 * @brief clears the cache
	 **/
	void ClearCache()
	{
		cache_->ClearCache();
	}

	Results results()
	{
		return results_;
	};

private:
	std::unique_ptr<CacheBase> cache_;
	CacheConf cache_conf_;
	// internal storage for the stack trace if needed
	StackTrace stack_trace_;
	StackTrace *stack_trace_ref_;
	Results results_;
};
