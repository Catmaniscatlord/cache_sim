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

#include <matplot/matplot.h>

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <ranges>
#include <thread>

#include "cache_sim.hpp"
#include "matplot/core/figure_registry.h"
#include "matplot/freestanding/axes_functions.h"
#include "matplot/freestanding/plot.h"
#include "matplot/util/common.h"
#include "util.hpp"

namespace po = boost::program_options;

void CreateOutputFiles(
	std::map<std::string, std::map<std::string, Results>> &results_map,
	std::string &output_folder);

void CreateOutputImages(
	std::map<std::string, std::map<std::string, Results>> &results_map,
	std::string &output_folder);

int main(int argc, char **argv)
{
	// Output folder for images and result files
	std::string output_folder;
	// Stack Traces. <trace,name>
	std::vector<std::pair<StackTrace, std::string>> st_arr;
	// Cache Configs. <config,name>
	std::vector<std::pair<CacheConf, std::string>> cc_arr;
	// Cache Sims
	std::vector<std::pair<CacheSim, std::string>> cs_arr;
	// [Stack trace][Cache config] results
	std::map<std::string, std::map<std::string, Results>> results_map;

	// clang-format off
	po::options_description desc{"Options"};
	desc.add_options()("help,h", "Help prompt")
		("stack-trace,s", po::value<std::vector<std::string>>()->multitoken()->composing(), "Stack Trace files")
		("cache-conf,c", po::value<std::vector<std::string>>()->multitoken()->composing(), "Cache Configuration files")
		( "output-folder,o", po::value<std::string>(), "Output Folder, defaults to '$CWD/output/'");
	// clang-format on

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);

	po::notify(vm);

	if (vm.count("stack-trace"))
	{
		for (const std::string &st_file :
			 vm["stack-trace"].as<std::vector<std::string>>())
		{
			auto st = Util::ReadStackTraceFile(st_file);
			if (st.has_value())
				st_arr.push_back(
					{st.value(), std::filesystem::path(st_file).filename()});
			else
			{
				std::cerr << "Stack Trace file " << st_file << " not found"
						  << std::endl;
				return 1;
			}
		}
	}

	if (vm.count("cache-conf"))
	{
		for (const std::string &cc_file :
			 vm["cache-conf"].as<std::vector<std::string>>())
		{
			auto cc = Util::ReadCacheConfFile(cc_file);
			if (cc.has_value())
				cc_arr.push_back(
					{cc.value(), std::filesystem::path(cc_file).filename()});
			else
			{
				std::cerr << "Cache Config file " << cc_file << " not found"
						  << std::endl;
				return 1;
			}
		}
	}

	if (vm.count("output-folder"))
		output_folder = vm["output-folder"].as<std::string>();
	else
		output_folder = "output/";

	if (!std::filesystem::exists(output_folder))
		std::filesystem::create_directory(output_folder);

	// Create the cache sims
	for (auto &cc : cc_arr)
		cs_arr.emplace_back(CacheSim(cc.first), cc.second);

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

	CreateOutputFiles(results_map, output_folder);
	CreateOutputImages(results_map, output_folder);
}

void CreateOutputFiles(
	std::map<std::string, std::map<std::string, Results>> &results_map,
	std::string &output_folder)
{
	for (auto &st_res : results_map)
	{
		for (auto &cc_res : st_res.second)
		{
			std::string output_file_name = output_folder + "/" + st_res.first +
										   "." + cc_res.first + ".out";
			std::ofstream output_file(
				std::move(output_file_name), std::ios::trunc | std::ios::out);
			if (!output_file)
				std::cerr << "error creating output file\n";
			Results res = cc_res.second;
			output_file << "Total Hit Rate\t : " << res.total_hit_rate
						<< std::endl;
			output_file << "Load Hit Rate\t : " << res.read_hit_rate
						<< std::endl;
			output_file << "Write Hit Rate\t : " << res.write_hit_rate
						<< std::endl;
			output_file << "Total Run Time\t : " << res.run_time << std::endl;
			output_file << "Average Memory Access Latency\t : "
						<< res.average_memory_access_time << std::endl;
		}
	}
}

void CreateOutputImages(
	std::map<std::string, std::map<std::string, Results>> &results_map,
	std::string &output_folder)
{
	// Table name, yaxis label, result value
	std::vector<
		std::pair<std::vector<std::string>, std::function<double(Results & r)>>>
		tables;

	tables.emplace_back(
		std::vector<std::string>{"Average Memory Access Time",
								 "Memory Access Time (clock cycles)"},
		[](Results &r) { return r.average_memory_access_time; });
	tables.emplace_back(
		std::vector<std::string>{"Total Hit Rate", "Total Hit Rate"},
		[](Results &r) { return r.total_hit_rate; });

	for (auto &table : tables)
	{
		matplot::vector_2d y;

		auto f = matplot::figure(true);
		f->size(1500, 1000);

		matplot::tiledlayout(2, 3);

		for (auto &stack_res : results_map)
		{
			matplot::nexttile();

			auto cache_res_v = stack_res.second | std::views::values |
							   std::views::transform(table.second);
			matplot::bar(
				std::vector<double>{cache_res_v.begin(), cache_res_v.end()});

			auto cache_names = std::views::keys(stack_res.second);
			matplot::xticklabels({cache_names.begin(), cache_names.end()});
			matplot::xtickangle(45.0);
			matplot::ylabel(table.first[1]);
			matplot::title(stack_res.first);
		}

		matplot::sgtitle(table.first[0]);

		matplot::save(output_folder + "/" + table.first[0], "svg");
	}
}
