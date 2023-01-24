#include <exception>
#include "game.h"

bool Grid::checkCell(Index ind) const {
	unsigned sum = 0;

    const auto row = static_cast<int>(ind.row);
    const auto col = static_cast<int>(ind.col);

	for (int i = row - 1; i <= row + 1; i++) {
		for (int j = col - 1; j <= col + 1; j++) {
			sum += at(getPeriodicIndex(i, j)).data;
		}
	}

	if (sum == 3) {
		return true;
	}
	else if (sum == 4) {
		return at(ind).data;
	}

	return false;
}

sf::Image createCellBordersImage(unsigned cell_size, sf::Color color) {
	sf::Image image;
	image.create(cell_size, cell_size, sf::Color::Transparent);

	for (unsigned x = 0; x < cell_size; x++) {
		image.setPixel(x, 0, color);
		image.setPixel(x, cell_size - 1, color);
	}
	for (unsigned y = 1; y < cell_size - 1; y++) {
		image.setPixel(0, y, color);
		image.setPixel(cell_size - 1, y, color);
	}

	return image;
}

sf::Image createSquareImage(unsigned size, sf::Color color) {
	sf::Image image;
	image.create(size, size, color);
	return image;
}

void GameOfLife::runStep() {
	for (std::size_t i = 0; i < rows_; i++) {
		for (std::size_t j = 0; j < columns_; j++) {
            const auto idx = Index{i, j};
			buffer_.at(idx) = { grid_.checkCell(idx) };
		}
	}
	grid_ = buffer_;
}

void GameOfLife::render(sf::RenderWindow& window) {
	if (is_hidden) {
		return;
	}

	const auto upper_left = sf::Vector2f(
        static_cast<float>(offset_x_),
        static_cast<float>(offset_y_)
    );
	resources_.alive_cell_sprite.setPosition(upper_left);
	resources_.dead_cell_sprite.setPosition(upper_left);

	for (std::size_t i = 0; i < rows_; i++) {
		for (std::size_t j = 0; j < columns_; j++) {
            const auto cur_pos = upper_left + sf::Vector2f{
                static_cast<float>(j * cell_size_ + 1),
                static_cast<float>(i * cell_size_ + 1)
            };

			if (grid_.at({ i, j }).data) {
				resources_.alive_cell_sprite.setPosition(cur_pos);
				window.draw(resources_.alive_cell_sprite);
			}
			else {
				resources_.dead_cell_sprite.setPosition(cur_pos);
				window.draw(resources_.dead_cell_sprite);
			}
		}
	}

	window.draw(resources_.grid_sprite);
}

void GameOfLife::handleClick(Position click_pos) {
	if (auto cell = getIndexFromPositionOnScreen(click_pos); cell) {
        auto& cell_value = grid_.at(*cell);
        cell_value.data = !cell_value.data;
	}
}

void GameOfLife::handleResize(unsigned int new_width, unsigned int new_height, sf::RenderWindow& window) {
	cell_size_ = static_cast<unsigned>(std::min(new_width / columns_, new_height / rows_));
	if (cell_size_ == 0) {
		is_hidden = true;
		return;
	}

	is_hidden = false;
	offset_x_ = (new_width - cell_size_ * static_cast<unsigned>(columns_)) / 2;
	offset_y_ = (new_height - cell_size_ * static_cast<unsigned>(rows_)) / 2;
	screen_width_ = new_width;
	screen_height_ = new_height;

	resetGridTexture();
	updateGridSprite();
	updateCellsScale();
}

void GameOfLife::resetGridTexture() {
	resources_.grid_image = createCellBordersImage(cell_size_);
	if (!resources_.grid_texture.create(cell_size_, cell_size_)) {
		throw std::runtime_error("could not create grid texture");
	}
	resources_.grid_texture.update(resources_.grid_image);
}

void GameOfLife::updateGridSprite() {
	resources_.grid_sprite.setTextureRect({
        0, 0,
        static_cast<int>(screen_width_ - 2 * offset_x_),
        static_cast<int>(screen_height_ - 2 * offset_y_)
    });
	const auto upper_left = sf::Vector2f(
        static_cast<float>(offset_x_),
        static_cast<float>(offset_y_)
    );
	resources_.grid_sprite.setPosition(upper_left);
}

void GameOfLife::updateCellsScale() {
    const auto factor = static_cast<float>(cell_size_ - 2);
	resources_.alive_cell_sprite.setScale(factor, factor);
	resources_.dead_cell_sprite.setScale(factor, factor);
}

void GameOfLife::initializeResources() {
	resources_.grid_texture.setSmooth(true);
	resources_.grid_texture.setRepeated(true);
	resources_.grid_sprite.setTexture(resources_.grid_texture);
	resources_.grid_sprite.setColor(default_grid);

	resources_.cell_image = createSquareImage(1, sf::Color::White);
	if (!resources_.cell_texture.create(1, 1)) {
		throw std::runtime_error("could not create cell texture");
	}
	resources_.cell_texture.update(resources_.cell_image);

	resources_.alive_cell_sprite.setTexture(resources_.cell_texture);
	resources_.dead_cell_sprite.setTexture(resources_.cell_texture);

	resources_.alive_cell_sprite.setColor(default_alive);
	resources_.dead_cell_sprite.setColor(default_dead);

	resetGridTexture();
	updateGridSprite();
	updateCellsScale();
}
