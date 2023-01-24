#include <imgui.h>
#include <imgui-SFML.h>
#include "utility.h"

void handleEvent(sf::RenderWindow& window, sf::Event& event, GameOfLife& game, Settings& settings) {
	ImGui::SFML::ProcessEvent(window, event);

	if (event.type == sf::Event::Closed) {
		window.close();
    } else if (event.type == sf::Event::Resized) {
        window.setView(sf::View{sf::FloatRect{
                0.f, 0.f,
                static_cast<float>(event.size.width),
                static_cast<float>(event.size.height)
        }});
        game.handleResize(event.size.width, event.size.height, window);
	} else if (!settings.in_menu && event.type == sf::Event::MouseButtonPressed
               && event.mouseButton.button == sf::Mouse::Left) {
		game.handleClick({
            static_cast<unsigned>(event.mouseButton.x),
            static_cast<unsigned>(event.mouseButton.y)
        });
	} else if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
        case sf::Keyboard::Escape:
            settings.in_menu = !settings.in_menu;
            break;
        case sf::Keyboard::Enter:
            settings.paused = !settings.paused;
            break;
        case sf::Keyboard::Left:
            settings.increaseSpeed();
            break;
        case sf::Keyboard::Right:
            settings.decreaseSpeed();
            break;
        default:
            break;
        }
    }
}

void drawMenu(sf::RenderWindow& window, GameOfLife& game, Settings& settings) {
	ImGui::SetNextWindowSize(ImVec2{ 400, 240 });

	const auto center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_None, ImVec2{ 0.5f, 0.5f });

    constexpr static ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize
                                                     | ImGuiWindowFlags_NoMove
                                                     | ImGuiWindowFlags_NoCollapse
                                                     | ImGuiWindowFlags_NoSavedSettings;
	ImGui::Begin(settings.paused ? "Menu - Paused" : "Menu - Running",
                 &settings.in_menu, window_flags);

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
	auto settings = Settings{};
    auto clock = sf::Clock{};
	int elapsed = 0;

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			handleEvent(window, event, game, settings);
		}

		ImGui::SFML::Update(window, clock.getElapsedTime());

		if (settings.in_menu) {
			drawMenu(window, game, settings);
		}

        if (!settings.paused) {
            elapsed += clock.restart().asMilliseconds();
            if (elapsed >= settings.step_delta_ms) {
                elapsed = 0;
                game.runStep();
            }
        }

		window.clear(settings.background_color.toSfColor());
		game.render(window);
		ImGui::SFML::Render(window);
		window.display();
	}
}

void runGame(RunOptions options) {
	auto window = sf::RenderWindow{
		sf::VideoMode(options.screen_width, options.screen_height),
		"Conway's Game of Life",
		options.fullscreen ? sf::Style::Fullscreen : sf::Style::Default
	};
	window.setVerticalSyncEnabled(true);
	if (!ImGui::SFML::Init(window)) {
        throw std::runtime_error("could not initialize ImGUI-SFML window");
    }

    // Disable ImGui automatic state saving
    ImGui::GetIO().IniFilename = nullptr;

	const auto [x, y] = window.getSize();
	auto game = GameOfLife({ 0, 0 }, x, y, options.cell_size);

	runGameLoop(window, game);

	ImGui::SFML::Shutdown();
}
