#include <cstdlib>
#include <tuple>
#include <cxxopts.hpp>
#include "options.h"

std::pair<unsigned, unsigned> getScreenDimensionsFromOption(std::string window_size) {
	const auto split_pos = window_size.find('x');
	const auto width_str = window_size.substr(0, split_pos);
	const auto height_str = window_size.substr(split_pos + 1);
	return std::make_pair(std::stoul(width_str), std::stoul(height_str));
}

RunOptions parseOptions(int argc, char** argv) {
	cxxopts::Options options("game_of_life", "Conway's Game of Life - cellular automata simulator");
	options.add_options()
		("f,fullscreen", "Run in fullscreen", cxxopts::value<bool>()->default_value("false"))
		("w,window", "Window size", cxxopts::value<std::string>()->default_value("1280x800"))
		("c,cell", "Grid cell size in pixels", cxxopts::value<unsigned>()->default_value("50"))
		("help", "Print application usage")
		;
	try {
		const auto opts_result = options.parse(argc, argv);
		if (opts_result.count("help")) {
			std::cout << options.help() << '\n';
			std::exit(EXIT_SUCCESS);
		}

		RunOptions result{};
		result.fullscreen = opts_result["fullscreen"].as<bool>();
		std::tie(result.screen_width, result.screen_height) = getScreenDimensionsFromOption(opts_result["window"].as<std::string>());
		result.cell_size = opts_result["cell"].as<unsigned>();

		return result;
	}
	catch (const cxxopts::OptionParseException& e) {
		std::cout << "Error: " << e.what();
		std::exit(EXIT_FAILURE);
	}
}
