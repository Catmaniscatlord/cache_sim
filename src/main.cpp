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

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>
#include <iostream>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char** argv)
{
	bool generate_graphs;
	// Stack Traces
	std::vector<StackTrace> st_arr;
	// Cache Configs
	std::vector<CacheConf> cc_arr;

	po::options_description desc{"Options"};
	desc.add_options()
		("help,h","Help prompt")
		("stack-trace,s", po::value<std::vector<std::string>>()->multitoken()->composing(), "Stack Trace files")
		("cache-conf,c", po::value<std::vector<std::string>>()->multitoken()->composing(), "Cache Configuration files")
	 ("create-graphs,g", po::bool_switch(&generate_graphs), "Create Graphs");

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).
					 options(desc).run(), vm);

	po::notify(vm);

	if (vm.count("stack-trace"))
	{
		for(const std::string& st_file : vm["stack-trace"].as<std::vector<std::string>>())
		{
			std::cerr << st_file << " ";
			auto st = Util::ReadStackTraceFile(st_file);
			if(st.has_value())
				st_arr.push_back(st.value());
			else
			{
				std::cerr << "Stack Trace file "  << st_file << " not found" << std::endl;
				return 1;
			}
		}
	}

	if (vm.count("cache-conf"))
	{
		for(const std::string& cc_file : vm["cache-conf"].as<std::vector<std::string>>())
		{
			std::cerr << cc_file << " ";
			auto cc = Util::ReadCacheConfFile(cc_file);
			if(cc.has_value())
				cc_arr.push_back(cc.value());
			else
			{
				std::cerr << "Cache Config file "  << cc_file << " not found" << std::endl;
				return 1;
			}
		}
	}

	auto cache_conf = Util::ReadCacheConfFile("../confs/4way-fifo.conf");

	if (!cache_conf.has_value())
	{
		std::cerr << "cache file not found" << std::endl;
		return 1;
	}

	auto stack_trace = Util::ReadStackTraceFile("../traces/mcf.trace");
	if (!stack_trace.has_value())
		std::cerr << "trace file not found" << std::endl;
	
	auto st = stack_trace.value();
	CacheSim cache_sim = CacheSim(cache_conf.value(), &st);
	cache_sim.set_stack_trace(&st);

	cache_sim.RunSimulation();


	Results res = cache_sim.results();

	std::cout << "Total Hit Rate\t : " << res.total_hit_rate << std::endl;
	std::cout << "Load Hit Rate\t : " << res.read_hit_rate << std::endl;
	std::cout << "Write Hit Rate\t : " << res.write_hit_rate << std::endl;
	std::cout << "Total Run Time\t : " << res.run_time << std::endl;
	std::cout << "Average Memory Access Latency\t : " << res.average_memory_access_time << std::endl;

}
