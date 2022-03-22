#include <imgui.h>
#include <imgui-SFML.h>
#include "utility.h"

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

void runGameLoop(sf::RenderWindow& window, GameOfLife& game) {
	Settings runtime_settings;
	sf::Clock clock;
	int elapsed_time = 0;

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			handleEvent(window, event, game, runtime_settings);
		}

		ImGui::SFML::Update(window, clock.getElapsedTime());

		if (runtime_settings.in_menu) {
			drawMenu(window, game, runtime_settings);
		}

		elapsed_time += clock.restart().asMilliseconds();
		if (elapsed_time >= runtime_settings.step_delta_ms) {
			elapsed_time = 0;
			if (!runtime_settings.paused) {
				game.runStep();
			}
		}

		window.clear(runtime_settings.background_color.toSfColor());
		game.render(window);
		ImGui::SFML::Render(window);
		window.display();
	}
}

void runGame(RunOptions options) {
	sf::RenderWindow window(
		sf::VideoMode(options.screen_width, options.screen_height),
		"Conway's Game of Life",
		options.fullscreen ? sf::Style::Fullscreen : sf::Style::Default
	);
	window.setVerticalSyncEnabled(true);
	ImGui::SFML::Init(window);

	const auto window_size = window.getSize();
	auto game = GameOfLife({ 0, 0 }, window_size.x, window_size.y, options.cell_size);

	runGameLoop(window, game);

	ImGui::SFML::Shutdown();
}
