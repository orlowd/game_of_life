#include <array>
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

struct RGBColor {
	RGBColor(float red, float green, float blue) :
		data_{ red, green, blue } {}
	
	RGBColor(sf::Color color) :
		data_{ color.r / 255.f, color.g / 255.f, color.b / 255.f } {}

	float red() const {
		return data_[RED];
	}
	float green() const {
		return data_[GREEN];
	}
	float blue() const {
		return data_[BLUE];
	}

	float* data() {
		return data_.data();
	}

	sf::Color toSfColor() const {
		return {
			static_cast<sf::Uint8>(red() * 255),
			static_cast<sf::Uint8>(green() * 255),
			static_cast<sf::Uint8>(blue() * 255)
		};
	}

private:
	std::array<float, 3> data_{};

	enum ColorChannel {
		RED = 0,
		GREEN = 1,
		BLUE = 2
	};
};


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
	bool pause = true;
	bool menu_open = true;

	int step_delta_ms = 500;
	constexpr static int max_update_ms = 4000;
	constexpr static int min_update_ms = 100;
	const int update_delta_ms = 100;

	std::array<RGBColor, 4> elements_colors{
		GameOfLife::default_grid_color,
		GameOfLife::default_alive_cells_color,
		GameOfLife::default_dead_cells_color,
		sf::Color::Black
	};

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
					if (step_delta_ms < max_update_ms) {
						step_delta_ms += update_delta_ms;
					}
					std::cout << "update_after_milliseconds = " << step_delta_ms << '\n';
				}
				else if (event.key.code == sf::Keyboard::Right) {
					if (step_delta_ms > min_update_ms) {
						step_delta_ms -= update_delta_ms;
					}
					std::cout << "update_after_milliseconds = " << step_delta_ms << '\n';
				}
			}
			else if (event.type == sf::Event::Resized) {
				window.setView(sf::View(sf::FloatRect{ 
					0.f, 0.f,
					static_cast<float>(event.size.width),
					static_cast<float>(event.size.height)
				}));
				game.handleResize(event.size.width, event.size.height, window);
			}
		}

		ImGui::SFML::Update(window, clock.getElapsedTime());

		if (menu_open) {
			constexpr static ImGuiWindowFlags window_flags =
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoSavedSettings;
			ImGui::SetNextWindowSize(ImVec2{ 400, 240 });
			const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_None, ImVec2{ 0.5f, 0.5f });
			ImGui::Begin("Menu", &menu_open, window_flags);
			const ImVec2 button_width = { ImGui::GetContentRegionAvail().x, 0 };
			if (ImGui::Button("Hide Menu", button_width)) {
				menu_open = false;
			}
			if (ImGui::Button(pause ? "Continue##pause" : "Pause##pause", button_width)) {
				pause = !pause;
			}
			if (ImGui::Button("Exit", button_width)) {
				window.close();
			}
			ImGui::Separator();
			ImGui::TextUnformatted("Colors:");
			ImGui::Indent();
			if (ImGui::ColorEdit3("Grid", elements_colors[0].data())) {
				game.setGridColor(elements_colors[0].toSfColor());
			}
			if (ImGui::ColorEdit3("Alive Cells", elements_colors[1].data())) {
				game.setAliveCellColor(elements_colors[1].toSfColor());
			}
			if (ImGui::ColorEdit3("Dead Cells", elements_colors[2].data())) {
				game.setDeadCellColor(elements_colors[2].toSfColor());
			}
			ImGui::ColorEdit3("Background", elements_colors[3].data());
			ImGui::Unindent();
			ImGui::SliderInt("Step Delay (ms)", &step_delta_ms, 1, 10'000, "%d", ImGuiSliderFlags_AlwaysClamp);
			ImGui::End();
		}

		elapsed_time += clock.restart().asMilliseconds();
		if (elapsed_time >= step_delta_ms) {
			elapsed_time = 0;
			if (!pause) {
				game.runStep();
			}
		}

		window.clear(elements_colors[3].toSfColor());
		game.render(window);
		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();

	return EXIT_SUCCESS;
}
