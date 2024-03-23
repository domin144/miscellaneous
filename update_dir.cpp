/*
 * SPDX-FileCopyrightText: 2023 Dominik Wójt <domin144@o2.pl>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <boost/program_options.hpp>
#include <exception>
#include <filesystem>
#include <iostream>

struct just_exit : std::exception
{
};

namespace po = boost::program_options;

struct options
{
	std::string source_directory;
	std::string target_directory;
	bool dry_run;
};

options parse_options(const int argc, const char* const argv[])
{
	po::options_description description("Allowed options");
	description.add_options() //
		("help", "show help") //
		("source-dir", po::value<std::string>(), "source directory") //
		("target-dir", po::value<std::string>(), "target directory") //
		("dry-run", "don't make the changes, just list them");

	po::positional_options_description positional_description;
	positional_description.add("source-dir", 1);
	positional_description.add("target-dir", 1);

	po::variables_map vm;
	po::store(
		po::command_line_parser(argc, argv)
			.options(description)
			.positional(positional_description)
			.run(),
		vm);
	po::notify(vm);

	if (vm.count("help"))
	{
		std::cout << description << std::endl;
		throw just_exit {};
	}

	const bool valid_command_line =
		vm.count("source-dir") == 1 && vm.count("target-dir") == 1;
	if (!valid_command_line)
	{
		std::cout << description << std::endl;
		throw std::runtime_error{"invalid arguments"};
	}

	options result;
	result.source_directory = vm["source-dir"].as<std::string>();
	result.target_directory = vm["target-dir"].as<std::string>();
	result.dry_run = vm.count("dry-run");
	return result;
}

int main(const int argc, const char* const argv[])
try
{
	const options options_0 = parse_options(argc, argv);

	std::cout << "from " << options_0.source_directory << " to "
			  << options_0.target_directory << '\n';
	if (options_0.dry_run)
	{
		std::cout << "dry run" << '\n';
	}

	return 0;
}
catch (just_exit&)
{
	return 1;
}
catch (std::exception& e)
{
	std::cerr << "standard exception caught: " << e.what() << std::endl;
	return 1;
}
