#pragma once

#include "game.h"
#include "options.h"
#include <array>
#include <SFML/Graphics.hpp>

struct RGBColor {
    constexpr RGBColor(float red, float green, float blue) : data_{red, green, blue} {}

    RGBColor(sf::Color color)
        : data_{color.r / 255.f, color.g / 255.f, color.b / 255.f} {}

    constexpr float red() const { return data_[RED]; }
    constexpr float green() const { return data_[GREEN]; }
    constexpr float blue() const { return data_[BLUE]; }

    constexpr float* data() { return data_.data(); }

    constexpr const float* data() const { return data_.data(); }

    sf::Color toSfColor() const {
        return {static_cast<sf::Uint8>(red() * 255),
                static_cast<sf::Uint8>(green() * 255),
                static_cast<sf::Uint8>(blue() * 255)};
    }

private:
    std::array<float, 3> data_;

    enum ColorChannel { RED = 0, GREEN = 1, BLUE = 2 };
};

struct Settings {
    bool paused = true;
    bool in_menu = true;
    int step_delta_ms = 400;
    RGBColor grid_color = {GameOfLife::default_grid};
    RGBColor alive_color = {GameOfLife::default_alive};
    RGBColor dead_color = {GameOfLife::default_dead};
    RGBColor background_color = {sf::Color::Black};

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

    static constexpr int max_update_ms = 10'000;
    static constexpr int min_update_ms = 1;
    static constexpr int update_delta_ms = 100;
};

void handleEvent(sf::RenderWindow& window,
                 sf::Event& event,
                 GameOfLife& game,
                 Settings& settings);
void drawMenu(sf::RenderWindow& window, GameOfLife& game, Settings& settings);
void runGameLoop(sf::RenderWindow& window, GameOfLife& game);
void runGame(RunOptions options);
