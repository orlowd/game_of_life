#include <array>
#include "catch.hpp"
#include "../src/options.h"

using namespace std::literals;

TEST_CASE("Screen resolution options are correctly parsed", "[options]") {
	{
		const auto in = "1920x1080"sv;
		const auto [width, height] = getScreenDimensionsFromOption(in);
		REQUIRE(width == 1920);
		REQUIRE(height == 1080);
	}
}

TEST_CASE("Command line options are correctly parsed", "[options]") {
	{
		std::array<std::string, 7> in{ "game_of_life", "-f", "-w", "1920x1080", "-c", "40", ""};
		std::array<char*, in.size()> argv{
			in[0].data(), in[1].data(), in[2].data(), in[3].data(),
			in[4].data(), in[5].data(), in[6].data()
		};

		const auto run_options = parseOptions(argv.size(), argv.data());
		REQUIRE(run_options.fullscreen == true);
		REQUIRE(run_options.screen_width == 1920);
		REQUIRE(run_options.screen_height == 1080);
		REQUIRE(run_options.cell_size == 40);
	}
}
