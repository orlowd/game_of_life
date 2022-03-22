#pragma once

#include <string>
#include <utility>

struct RunOptions {
	bool fullscreen;
	unsigned screen_width;
	unsigned screen_height;
	unsigned cell_size;
};

std::pair<unsigned, unsigned> getScreenDimensionsFromOption(std::string window_size);

RunOptions parseOptions(int argc, char** argv);
