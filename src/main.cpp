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

#include <algorithm>
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>
#include <filesystem>
#include <fstream>
#include <future>
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

	/************************
	 * Command line options *
	 ************************/

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

	std::vector<std::pair<std::future<std::optional<StackTrace>>, std::string>>
		st_read_files;
	if (vm.count("stack-trace"))
	{
		// start multithreaded read
		for (const std::string &st_file :
			 vm["stack-trace"].as<std::vector<std::string>>())
		{
			st_read_files.emplace_back(
				std::async(std::launch::async,
						   [=]() { return Util::ReadStackTraceFile(st_file); }),
				st_file);
		}
	}

	// joins the futures
	for (auto &st : st_read_files)
	{
		auto read_st = st.first.get();
		if (read_st.has_value())
			st_arr.emplace_back(std::move(read_st.value()),
								std::filesystem::path(st.second).filename());
		else
		{
			std::cerr << "Stack Trace file " << st.second << " not found"
					  << std::endl;
			return 1;
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

	/*********************************
	 * Running The Cache Simulations *
	 *********************************/
	// Create the cache sims
	for (auto &cc : cc_arr)
		cs_arr.emplace_back(CacheSim(cc.first), cc.second);

	// multithreading go brrt
	std::vector<std::jthread> sim_threads;
	for (auto &cs : cs_arr)
	{
		// we don't edit the stack trace so we wont have concurrency issues
		// if we multithread by cache
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

	// join up our simulation threads
	for (auto &i : sim_threads)
		i.join();

	/******************
	 * Output Results *
	 ******************/
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
	using namespace matplot;

	// Table name, y-axis label, result value
	std::vector<
		std::pair<std::vector<std::string>, std::function<double(Results & r)>>>
		tables;

	// average memory access time
	tables.emplace_back(
		std::vector<std::string>{"Average-Memory-Access-Time",
								 "Average Memory Access Time (clock cycles)"},
		[](Results &r) { return r.average_memory_access_time; });

	// total hit rate
	tables.emplace_back(
		std::vector<std::string>{"Total-Hit-Rate", "Total Hit Rate"},
		[](Results &r) { return r.total_hit_rate; });

	// creates the table for each output type listed in the tables vector
	for (auto &table : tables)
	{
		for (auto &stack_res : results_map)
		{
			auto f = figure();
			f->quiet_mode(true);

			auto cache_res_v = stack_res.second | std::views::values |
							   std::views::transform(table.second);

			vector_1d y =
				std::vector<double>{cache_res_v.begin(), cache_res_v.end()};
			auto b = bar(y);

			std::vector<double> label_x;
			std::vector<double> label_y;
			std::vector<std::string> labels;
			double max = *std::max_element(y.begin(), y.end());
			for (size_t i = 0; i < y.size(); ++i)
			{
				label_x.emplace_back(b->x_end_point(i, 0) * 1.21 - 0.35);
				label_y.emplace_back(y[i] + (max * .05));
				std::stringstream ss;
				ss << std::setprecision(3) << y[i];
				labels.emplace_back(std::move(ss.str()));
			}
			hold(on);
			text(label_x, label_y, labels)->font("Times New Roman");

			auto cache_names =
				std::views::keys(stack_res.second) |
				std::views::transform(
					[](std::string s) {
						return s.erase(s.find_last_of("."), std::string::npos);
					});
			xticklabels({cache_names.begin(), cache_names.end()});
			xtickangle(24.0);
			ylabel(table.first[1]);
			title(stack_res.first);
			save(output_folder + table.first[0] + "-" + stack_res.first +
				 ".jpg");
		}
	}
}
