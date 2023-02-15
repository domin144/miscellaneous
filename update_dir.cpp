#include <boost/program_options.hpp>
#include <filesystem>
#include <iostream>

namespace po = boost::program_options;

int main(const int argc, const char* const argv[])
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
			.positional(positional_description).run(),
		vm);
	po::notify(vm);

	if (vm.count("help"))
	{
		std::cout << description << std::endl;
		return 1;
	}

	const bool valid_command_line =
		vm.count("source-dir") == 1 && vm.count("target-dir") == 1;
	if (!valid_command_line)
	{
		std::cerr << "invalid parameters" << std::endl;
		std::cout << description << std::endl;
		return 1;
	}

	std::string source = vm["source-dir"].as<std::string>();
	std::string target = vm["target-dir"].as<std::string>();

	std::cout << "from " << source << " to " << target << std::endl;

	return 0;
}
