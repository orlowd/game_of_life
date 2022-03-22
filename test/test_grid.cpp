#include "catch.hpp"
#include "../src/game.h"

TEST_CASE("Grid class sizes are correct", "[grid]") {
	{
		const Grid grid{5, 10};
		REQUIRE(grid.columns() == 5);
		REQUIRE(grid.rows() == 10);
		const auto size = grid.getSize();
		REQUIRE(size.col == 5);
		REQUIRE(size.row == 10);
	}
}
