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
	constexpr RGBColor(float red, float green, float blue) :
		data_{ red, green, blue } {}
	
	RGBColor(sf::Color color) :
		data_{ color.r / 255.f, color.g / 255.f, color.b / 255.f } {}

	constexpr float red() const {
		return data_[RED];
	}
	constexpr float green() const {
		return data_[GREEN];
	}
	constexpr float blue() const {
		return data_[BLUE];
	}

	constexpr float* data() {
		return data_.data();
	}

	constexpr const float* data() const {
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
	std::array<float, 3> data_;

	enum ColorChannel {
		RED = 0,
		GREEN = 1,
		BLUE = 2
	};
};

struct Settings {
	bool paused{ true };
	bool in_menu{ true };
	int step_delta_ms{ 400 };
	RGBColor grid_color{ GameOfLife::default_grid_color };
	RGBColor alive_color{ GameOfLife::default_alive_cells_color };
	RGBColor dead_color{ GameOfLife::default_dead_cells_color };
	RGBColor background_color{ sf::Color::Black };

	void increaseSpeed() {
		if (step_delta_ms <= max_update_ms - update_delta_ms) {
			step_delta_ms += update_delta_ms;
		}
	}

	void decreaseSpeed() {
		if (step_delta_ms >= min_update_ms + update_delta_ms) {
			step_delta_ms -= update_delta_ms;
		}
	}

	constexpr static int max_update_ms = 10'000;
	constexpr static int min_update_ms = 1;
	constexpr static int update_delta_ms = 100;
};

void handleEvent(sf::RenderWindow& window, sf::Event& event, GameOfLife& game, Settings& settings) {
	ImGui::SFML::ProcessEvent(window, event);
	if (event.type == sf::Event::Closed) {
		window.close();
	}
	else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
		settings.in_menu = !settings.in_menu;
	}
	else if (!settings.in_menu && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
		game.handleClick({ static_cast<unsigned>(event.mouseButton.x), static_cast<unsigned>(event.mouseButton.y) });
	}
	else if (event.type == sf::Event::KeyPressed) {
		if (event.key.code == sf::Keyboard::Enter) {
			settings.paused = !settings.paused;
		}
		else if (event.key.code == sf::Keyboard::Left) {
			settings.increaseSpeed();
		}
		else if (event.key.code == sf::Keyboard::Right) {
			settings.decreaseSpeed();
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

void drawMenu(sf::RenderWindow& window, GameOfLife& game, Settings& settings) {
	constexpr static ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoSavedSettings;
	ImGui::SetNextWindowSize(ImVec2{ 400, 240 });
	const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_None, ImVec2{ 0.5f, 0.5f });
	ImGui::Begin(settings.paused ? "Menu - Paused" : "Menu - Running", &settings.in_menu, window_flags);
	const ImVec2 button_width = { ImGui::GetContentRegionAvail().x, 0 };
	if (ImGui::Button("Hide Menu", button_width)) {
		settings.in_menu = false;
	}
	if (ImGui::Button(settings.paused ? "Continue##pause" : "Pause##pause", button_width)) {
		settings.paused = !settings.paused;
	}
	if (ImGui::Button("Exit", button_width)) {
		window.close();
	}
	ImGui::Separator();
	ImGui::TextUnformatted("Colors:");
	ImGui::Indent();
	if (ImGui::ColorEdit3("Grid", settings.grid_color.data())) {
		game.setGridColor(settings.grid_color.toSfColor());
	}
	if (ImGui::ColorEdit3("Alive Cells", settings.alive_color.data())) {
		game.setAliveCellColor(settings.alive_color.toSfColor());
	}
	if (ImGui::ColorEdit3("Dead Cells", settings.dead_color.data())) {
		game.setDeadCellColor(settings.dead_color.toSfColor());
	}
	ImGui::ColorEdit3("Background", settings.background_color.data());
	ImGui::Unindent();
	ImGui::SliderInt("Step Delay (ms)", &settings.step_delta_ms,
		Settings::min_update_ms, Settings::max_update_ms,
		"%d", ImGuiSliderFlags_AlwaysClamp);
	ImGui::End();
}

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
	Settings settings;

	int elapsed_time = 0;
	sf::Clock clock;
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			handleEvent(window, event, game, settings);
		}

		ImGui::SFML::Update(window, clock.getElapsedTime());

		if (settings.in_menu) {
			drawMenu(window, game, settings);
		}

		elapsed_time += clock.restart().asMilliseconds();
		if (elapsed_time >= settings.step_delta_ms) {
			elapsed_time = 0;
			if (!settings.paused) {
				game.runStep();
			}
		}

		window.clear(settings.background_color.toSfColor());
		game.render(window);
		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();

	return EXIT_SUCCESS;
}
