/**
 * filename: cache_factory.cpp
 *
 * description: object file for a cache
 *
 * authors: Chamberlain, David
 **/

#include "cache.hpp"
#include "fifo_cache.hpp"
#include "rand_cache.hpp"

namespace CacheFactory
{
std::unique_ptr<CacheBase> CreateCache(const CacheConf& cc)
{
	switch (cc.replacement_policy_)
	{
		case RAND:
			return std::make_unique<RandCache>(cc);
			break;
		case FIFO:
			return std::make_unique<FifoCache>(cc);
			break;
	}
	return nullptr;
}
};	// namespace CacheFactory
