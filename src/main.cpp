#include <cassert>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <cxxopts.hpp>
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

#include "game.h"


struct RunOptions {
	bool fullscreen;
	unsigned screen_width;
	unsigned screen_height;
	unsigned cell_size;
};

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


int main(int argc, char** argv)
{
	const auto opts = parseOptions(argc, argv);

	sf::RenderWindow window(
		sf::VideoMode(opts.screen_width, opts.screen_height),
		"Conway's Game of Life",
		(opts.fullscreen) ? sf::Style::Fullscreen : sf::Style::Default
	);
	window.setVerticalSyncEnabled(true);
	ImGui::SFML::Init(window);

	const auto window_size = window.getSize();
	auto game = GameOfLife({ 0, 0 }, window_size.x, window_size.y, opts.cell_size);
	bool pause = true;
	bool menu_open = true;

	int update_after_milliseconds = 500;
	const int max_update_ms = 4000;
	const int min_update_ms = 100;
	const int update_delta_ms = 100;

	int elapsed_time = 0;
	sf::Clock clock;
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			ImGui::SFML::ProcessEvent(window, event);
			if (event.type == sf::Event::Closed) {
				window.close();
			}
			else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
				menu_open = !menu_open;
			}
			else if (!menu_open && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
				game.handleClick({ static_cast<unsigned>(event.mouseButton.x), static_cast<unsigned>(event.mouseButton.y) });
			}
			else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Enter) {
					pause = !pause;
					std::cout << "pause = " << pause << '\n';
				}
				else if (event.key.code == sf::Keyboard::Left) {
					if (update_after_milliseconds < max_update_ms) {
						update_after_milliseconds += update_delta_ms;
					}
					std::cout << "update_after_milliseconds = " << update_after_milliseconds << '\n';
				}
				else if (event.key.code == sf::Keyboard::Right) {
					if (update_after_milliseconds > min_update_ms) {
						update_after_milliseconds -= update_delta_ms;
					}
					std::cout << "update_after_milliseconds = " << update_after_milliseconds << '\n';
				}
			}
			else if (event.type == sf::Event::Resized) {
				window.setView(sf::View(sf::FloatRect{ 
					0.f, 0.f,
					static_cast<float>(event.size.width),
					static_cast<float>(event.size.height)
				}));
				game.handleResize(event.size.width, event.size.height, window);
				window.clear(sf::Color::Black);
			}
		}

		ImGui::SFML::Update(window, clock.getElapsedTime());

		if (menu_open) {
			ImGui::Begin("Menu", &menu_open);
			if (ImGui::Button("Hide Menu")) {
				menu_open = false;
			}
			else if (ImGui::Button("Pause")) {
				pause = !pause;
			}
			else if (ImGui::Button("Exit")) {
				window.close();
			}
			ImGui::End();
		}

		elapsed_time += clock.restart().asMilliseconds();
		if (elapsed_time >= update_after_milliseconds) {
			elapsed_time = 0;
			if (!pause) {
				game.runStep();
			}
		}

		window.clear();
		game.render(window);
		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();

	return EXIT_SUCCESS;
}
