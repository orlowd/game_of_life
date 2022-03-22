#pragma once

#include <cassert>
#include <cstdlib>
#include <optional>
#include <vector>
#include <SFML/Graphics.hpp>

struct Position {
	unsigned x;
	unsigned y;
};

struct Index {
	std::size_t row;
	std::size_t col;
};

struct Cell {
	bool data;
};

class Grid {
public:
	Grid(std::size_t columns, std::size_t rows)
		: grid_{ rows, std::vector(columns, Cell{false}) }
	{
		assert(columns > 0 && rows > 0);
	}

	std::size_t rows() const {
		return grid_.size();
	}

	std::size_t columns() const {
		return grid_[0].size();
	}

	Index getSize() const {
		return { .row = rows(), .col = columns() };
	}

	Cell at(Index ind) const {
		return grid_[ind.row][ind.col];
	}
	Cell& at(Index ind) {
		return grid_[ind.row][ind.col];
	}

	Index getPeriodicIndex(int row, int col) const {
		if (row < 0) {
			row = static_cast<int>(rows() + row);
		}
		if (col < 0) {
			col = static_cast<int>(columns() + col);
		}
		return { .row = row % rows(), .col = col % columns() };
	}

	bool checkCell(Index ind) const;
private:
	std::vector<std::vector<Cell>> grid_;
};

sf::Image createCellBordersImage(unsigned cell_size, sf::Color color = sf::Color::White);
sf::Image createSquareImage(unsigned size, sf::Color color = sf::Color::White);

class GameOfLife {
public:
	GameOfLife(Position upper_left, unsigned screen_width, unsigned screen_height, unsigned cell_size)
		: start_pos_{ upper_left }, screen_width_{ screen_width }, screen_height_{ screen_height }, cell_size_{ cell_size },
		columns_{ screen_width / cell_size }, rows_{ screen_height / cell_size }, grid_{ columns_, rows_ }, buffer_{ grid_ },
		offset_x_{ (screen_width - cell_size * static_cast<unsigned>(columns_)) / 2 },
		offset_y_{ (screen_height - cell_size * static_cast<unsigned>(rows_)) / 2 }
	{
		initializeResources();
	}

	void runStep();
	void render(sf::RenderWindow& window);

	void handleClick(Position click_pos);
	void handleResize(unsigned int new_width, unsigned int new_height, sf::RenderWindow& window);

	void setGridColor(sf::Color new_color) {
		resources_.grid_sprite.setColor(new_color);
	}
	void setAliveCellColor(sf::Color new_color) {
		resources_.alive_cell_sprite.setColor(new_color);
	}
	void setDeadCellColor(sf::Color new_color) {
		resources_.dead_cell_sprite.setColor(new_color);
	}

	inline const static sf::Color default_grid_color{ 128, 128, 128 };
	inline const static sf::Color default_alive_cells_color{ sf::Color::White };
	inline const static sf::Color default_dead_cells_color{ sf::Color::Black };
private:
	struct Configuration {
		sf::Color grid_color{ sf::Color::White };
		sf::Color alive_color{ sf::Color::White };
		sf::Color dead_color{ sf::Color::Black };
		unsigned speed{ 1u };
	};
	struct Resources {
		sf::Image grid_image;
		sf::Texture grid_texture;
		sf::Sprite grid_sprite;

		sf::Image cell_image;
		sf::Texture cell_texture;
		sf::Sprite alive_cell_sprite;
		sf::Sprite dead_cell_sprite;
	};

	std::optional<Index> getIndexFromPositionOnScreen(Position pos) const {
		if (isOutOfScreenBounds(pos)) {
			return std::nullopt;
		}
		const Position relative = { pos.x - start_pos_.x - offset_x_, pos.y - start_pos_.y - offset_y_ };
		return Index{ relative.y / cell_size_, relative.x / cell_size_ };
	}

	bool isOutOfScreenBounds(Position pos) const {
		return (
			pos.x < start_pos_.x + offset_x_ ||
			pos.y < start_pos_.y + offset_y_ ||
			pos.x >= start_pos_.x + cell_size_ * grid_.columns() + offset_x_ ||
			pos.y >= start_pos_.y + cell_size_ * grid_.rows() + offset_y_
			);
	}

	void resetGridTexture();
	void updateGridSprite();
	void updateCellsScale();
	void initializeResources();

	Position start_pos_;
	unsigned screen_width_;
	unsigned screen_height_;
	unsigned cell_size_;

	std::size_t columns_;
	std::size_t rows_;

	unsigned offset_x_;
	unsigned offset_y_;

	Grid grid_;
	Grid buffer_;

	Configuration config_;
	Resources resources_;

	bool is_hidden = false;
};
