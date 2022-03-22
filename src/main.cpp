#include <cstdlib>

#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>

#include "options.h"
#include "game.h"
#include "utility.h"


int main(int argc, char** argv)
{
	const auto opts = parseOptions(argc, argv);

	sf::RenderWindow window(
		sf::VideoMode(opts.screen_width, opts.screen_height),
		"Conway's Game of Life",
		opts.fullscreen ? sf::Style::Fullscreen : sf::Style::Default
	);
	window.setVerticalSyncEnabled(true);
	ImGui::SFML::Init(window);

	const auto window_size = window.getSize();
	auto game = GameOfLife({ 0, 0 }, window_size.x, window_size.y, opts.cell_size);
	
	runGameLoop(window, game);

	ImGui::SFML::Shutdown();

	return EXIT_SUCCESS;
}
