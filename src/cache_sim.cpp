/**
 * filename: cache_sim.cpp
 *
 * description: loads config and trace files into the cache simulator
 *
 * authors: Chamberlain, David
 *
 **/

#include "cache_sim.hpp"

#include <cstdint>

Results CacheSimulator::SimulateTrace(const StackTrace& st)
{
	// memory access count
	const auto ac{st.size()};

	uint64_t rc{};	// read count
	uint64_t wc{};	// write count
	uint64_t ic{};	// instruction count
	uint64_t rm{};	// read misses
	uint64_t wm{};	// write misses

	for (auto& ma : st)
	{
		if (ma.is_read)
			rc++;
		else
			wc++;
		ic += ma.last_memory_access_count + 1;

		// if miss
		if (!cache_->AccessMemory(ma.address, ma.is_read))
		{
			if (ma.is_read)
				rm++;
			else
				wm++;
		}
	}

	return {.total_hit_rate =
				1.0f - static_cast<double>(rm + wm) / static_cast<double>(ac),
			.read_hit_rate =
				1.0f - static_cast<double>(rm) / static_cast<double>(rc),
			.write_hit_rate =
				1.0f - static_cast<double>(wm) / static_cast<double>(wc),
			.run_time = ic + (rm + wm) * cache_conf_.miss_penalty_,
			.average_memory_access_time =
				1 + (static_cast<double>(rm + wm) / static_cast<double>(ac)) *
						cache_conf_.miss_penalty_};
}
