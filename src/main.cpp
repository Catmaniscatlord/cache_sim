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

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>
#include <filesystem>
#include <iostream>
#include <thread>
#include <unordered_map>

#include "cache_sim.hpp"
#include "util.hpp"

namespace po = boost::program_options;

int main(int argc, char **argv)
{
	bool generate_graphs;
	// Stack Traces. <trace,name>
	std::vector<std::pair<StackTrace, std::string>> st_arr;
	// Cache Configs. <config,name>
	std::vector<std::pair<CacheConf, std::string>> cc_arr;
	// Cache Sims
	std::vector<std::pair<CacheSim, std::string>> cs_arr;

	// [Stack trace][Cache config] results
	std::unordered_map<std::string, std::unordered_map<std::string, Results>> results_map;

	po::options_description desc{"Options"};
	desc.add_options()("help,h", "Help prompt")(
		"stack-trace,s", po::value<std::vector<std::string>>()->multitoken()->composing(),
		"Stack Trace files")(
		"cache-conf,c", po::value<std::vector<std::string>>()->multitoken()->composing(),
		"Cache Configuration files")(
		"create-graphs,g", po::bool_switch(&generate_graphs), "Create Graphs");

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);

	po::notify(vm);

	if (vm.count("stack-trace"))
	{
		for (const std::string &st_file : vm["stack-trace"].as<std::vector<std::string>>())
		{
			auto st = Util::ReadStackTraceFile(st_file);
			if (st.has_value())
				st_arr.push_back({st.value(), std::filesystem::path(st_file).filename()});
			else
			{
				std::cerr << "Stack Trace file " << st_file << " not found" << std::endl;
				return 1;
			}
		}
	}

	if (vm.count("cache-conf"))
	{
		for (const std::string &cc_file : vm["cache-conf"].as<std::vector<std::string>>())
		{
			auto cc = Util::ReadCacheConfFile(cc_file);
			if (cc.has_value())
				cc_arr.push_back({cc.value(), std::filesystem::path(cc_file).filename()});
			else
			{
				std::cerr << "Cache Config file " << cc_file << " not found" << std::endl;
				return 1;
			}
		}
	}

	// Create the cache sims
	for (auto &cc : cc_arr)
		cs_arr.push_back({CacheSim(cc.first), cc.second});

	std::vector<std::jthread> sim_threads;
	for (auto &cs : cs_arr)
	{
		sim_threads.push_back(std::jthread(
			[&]()
			{
				for (auto &st : st_arr)
				{
					cs.first.set_stack_trace(&st.first);
					cs.first.RunSimulation();
					cs.first.ClearCache();
					results_map[st.second][cs.second] = cs.first.results();
				}
			}));
	}

	for (auto &i : sim_threads)
		i.join();

	for (auto &i : results_map)
	{
		std::cout << "stack trace " << i.first << std::endl;
		for (auto j : i.second)
		{
			// std::cout << "cache config" << j.first << std::endl;
			// auto res = j.second;
			// std::cout << "Total Hit Rate\t : " << res.total_hit_rate << std::endl;
			// std::cout << "Load Hit Rate\t : " << res.read_hit_rate << std::endl;
			// std::cout << "Write Hit Rate\t : " << res.write_hit_rate << std::endl;
			// std::cout << "Total Run Time\t : " << res.run_time << std::endl;
			// std::cout << "Average Memory Access Latency\t : " <<
			// res.average_memory_access_time << std::endl;
		}
	}
}
