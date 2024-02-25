/**
 * filename: main.cpp
 *
 * description: loads config and trace files into the cache simulator
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
#include "util.hpp"

#include <iostream>

// int main(int argc, char** argv)
int main()
{
	auto cache_conf = Util::ReadCacheFile("../confs/4way-fifo.conf");

	if (!cache_conf.has_value())
		std::cerr << "cache file not found" << std::endl;

	auto stack_trace = Util::ReadStackTraceFile("../traces/mcf.trace");
	if (!stack_trace.has_value())
		std::cerr << "trace file not found" << std::endl;

	CacheSim cache_sim = CacheSim(cache_conf.value(), std::move(stack_trace.value()));
	cache_sim.RunSimulation();
	Results res = cache_sim.results();

	std::cout << "Total Hit Rate\t : " << res.total_hit_rate << std::endl;
	std::cout << "Load Hit Rate\t : " << res.read_hit_rate << std::endl;
	std::cout << "Write Hit Rate\t : " << res.write_hit_rate << std::endl;
	std::cout << "Total Run Time\t : " << res.run_time << std::endl;
	std::cout << "Average Memory Access Latency\t : " << res.average_memory_access_time << std::endl;
}
