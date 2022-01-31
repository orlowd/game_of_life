#include <cassert>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <cxxopts.hpp>
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

	bool checkCell(Index ind) const {
		unsigned sum = 0;

		for (int i = static_cast<int>(ind.row) - 1; i <= static_cast<int>(ind.row) + 1; i++) {
			for (int j = static_cast<int>(ind.col) - 1; j <= static_cast<int>(ind.col) + 1; j++) {
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
private:
	std::vector<std::vector<Cell>> grid_;
};

sf::Image createCellBordersImage(unsigned cell_size, sf::Color color = sf::Color::White)
{
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

sf::Image createSquareImage(unsigned size, sf::Color color = sf::Color::White)
{
	sf::Image image;
	image.create(size, size, color);
	return image;
}

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

	void runStep() {
		for (std::size_t i = 0; i < rows_; i++) {
			for (std::size_t j = 0; j < columns_; j++) {
				buffer_.at(Index{ i, j }) = { grid_.checkCell(Index{ i, j }) };
			}
		}
		grid_ = buffer_;
	}

	void render(sf::RenderWindow& window) {
		const sf::Vector2f upper_left{ static_cast<float>(offset_x_), static_cast<float>(offset_y_) };
		resources_.alive_cell_sprite.setPosition(upper_left);
		resources_.dead_cell_sprite.setPosition(upper_left);
		for (std::size_t i = 0; i < rows_; i++) {
			for (std::size_t j = 0; j < columns_; j++) {
				if (grid_.at({ i, j }).data == true) {
					resources_.alive_cell_sprite.setPosition(
						upper_left + sf::Vector2f{ static_cast<float>(j * cell_size_ + 1), static_cast<float>(i * cell_size_ + 1) }
					);
					window.draw(resources_.alive_cell_sprite);
				}
				else {
					resources_.dead_cell_sprite.setPosition(
						upper_left + sf::Vector2f{ static_cast<float>(j * cell_size_ + 1), static_cast<float>(i * cell_size_ + 1) }
					);
					window.draw(resources_.dead_cell_sprite);
				}
			}
		}
		window.draw(resources_.grid_sprite);
	}

	void handleClick(Position click_pos) {
		auto cell = getIndexFromPositionOnScreen(click_pos);
		if (!cell) {
			return;
		}
		auto& cell_value = grid_.at(*cell);
		cell_value.data = !cell_value.data;
	}

	void handleResize(unsigned int new_width, unsigned int new_height, sf::RenderWindow& window) {
		cell_size_ = static_cast<unsigned>(std::min(new_width / columns_, new_height / rows_));
		offset_x_ = (new_width - cell_size_ * static_cast<unsigned>(columns_)) / 2;
		offset_y_ = (new_height - cell_size_ * static_cast<unsigned>(rows_)) / 2;
		screen_width_ = new_width;
		screen_height_ = new_height;

		resetGridTexture();
		updateGridSprite();
		updateCellsScale();
	}

private:
	struct Configuration {
		sf::Color grid_color{sf::Color::White};
		sf::Color alive_color{sf::Color::White};
		sf::Color dead_color{sf::Color::Black};
		unsigned speed{1u};		// in what units should speed be specified, how to convert into a representable format, how to work with it inside a game logic??
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

	void resetGridTexture() {
		resources_.grid_image = createCellBordersImage(cell_size_);
		if (!resources_.grid_texture.create(cell_size_, cell_size_)) {
			std::cout << "ERROR: could not create grid texture\n";		// should rather throw
		}
		resources_.grid_texture.update(resources_.grid_image);
	}

	void updateGridSprite() {
		resources_.grid_sprite.setTextureRect(sf::IntRect{ 0, 0, static_cast<int>(screen_width_ - 2 * offset_x_), static_cast<int>(screen_height_ - 2 * offset_y_) });
		const sf::Vector2f upper_left{ static_cast<float>(offset_x_), static_cast<float>(offset_y_) };
		resources_.grid_sprite.setPosition(upper_left);
	}

	void updateCellsScale() {
		resources_.alive_cell_sprite.setScale(static_cast<float>(cell_size_ - 2), static_cast<float>(cell_size_ - 2));
		resources_.dead_cell_sprite.setScale(static_cast<float>(cell_size_ - 2), static_cast<float>(cell_size_ - 2));
	}

	void initializeResources() {
		resources_.grid_texture.setSmooth(true);
		resources_.grid_texture.setRepeated(true);
		resources_.grid_sprite.setTexture(resources_.grid_texture);
		resources_.grid_sprite.setColor(sf::Color{ 128, 128, 128 });

		resources_.cell_image = createSquareImage(1, sf::Color::White);
		if (!resources_.cell_texture.create(1, 1)) {
			std::cout << "ERROR: could not create cell texture\n";		// should rather throw
		}
		resources_.cell_texture.update(resources_.cell_image);

		resources_.alive_cell_sprite.setTexture(resources_.cell_texture);
		resources_.dead_cell_sprite.setTexture(resources_.cell_texture);

		resources_.alive_cell_sprite.setColor(sf::Color::White);
		resources_.dead_cell_sprite.setColor(sf::Color::Black);

		resetGridTexture();
		updateGridSprite();
		updateCellsScale();
	}

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
};


int main(int argc, char** argv)
{
	cxxopts::Options options("Conway's Game Of Life", "A world's famous cellular automata simulator");
	options.add_options()
		("f,fullscreen", "Run in fullscreen", cxxopts::value<bool>()->default_value("false"))
		("w,window", "Window size", cxxopts::value<std::string>()->default_value("640x800"))
		("c,cell", "Grid cell size in pixels", cxxopts::value<unsigned>()->default_value("50"))
		;
	auto opt_result = options.parse(argc, argv);	// TODO: finish parsing logic

	sf::RenderWindow window(sf::VideoMode(1280, 800), "Hello SFML World!");
	window.setVerticalSyncEnabled(true);

	const auto window_size = window.getSize();
	auto game = GameOfLife({ 0, 0 }, window_size.x, window_size.y, 50);
	bool pause = true;

	int update_after_milliseconds = 500;
	const int max_update_ms = 4000;
	const int min_update_ms = 100;
	const int update_delta_ms = 100;

	int elapsed_time = 0;
	sf::Clock clock;
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed || 
				event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
				window.close();
			}
			else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
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

		elapsed_time += clock.restart().asMilliseconds();
		if (elapsed_time >= update_after_milliseconds) {
			elapsed_time = 0;
			if (!pause) {
				game.runStep();
			}
		}

		window.clear();
		game.render(window);
		window.display();
	}

	return 0;
}
