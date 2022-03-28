#include "catch.hpp"
#include "../src/game.h"

TEST_CASE("Grid class sizes are correct", "[grid]") {
	{
		const Grid grid{ 5, 10 };
		REQUIRE(grid.rows() == 5);
		REQUIRE(grid.columns() == 10);
		const auto size = grid.getSize();
		REQUIRE(size.row == 5);
		REQUIRE(size.col == 10);
	}
}

TEST_CASE("Grid class indexing logic is correct", "[grid]") {
	{
		Grid grid{ 5, 10 };
		REQUIRE(grid.at({ 3, 4 }).data == false);
		grid.at({ 1, 3 }).data = true;
		grid.at({ 0, 0 }).data = true;
		grid.at({ 4, 9 }).data = true;
		REQUIRE(grid.at({ 1, 3 }).data == true);
		REQUIRE(grid.at({ 0, 0 }).data == true);
		REQUIRE(grid.at({ 4, 9 }).data == true);
		grid.at({ 1, 3 }).data = false;
		REQUIRE(grid.at({ 1, 3 }).data == false);
	}
}

TEST_CASE("Grid class periodic indexing is correct", "[grid]") {
	{
		Grid grid{ 4, 5 };
		const Index index{ 2, 4 };
		const Index same_index = grid.getPeriodicIndex(index.row, index.col);
		REQUIRE(same_index.row == index.row);
		REQUIRE(same_index.col == index.col);
		const Index zero_index = grid.getPeriodicIndex(0, 0);
		REQUIRE(zero_index.row == 0);
		REQUIRE(zero_index.col == 0);
		const Index period_index = grid.getPeriodicIndex(5, 7);
		REQUIRE(period_index.row == 1);
		REQUIRE(period_index.col == 2);
		const Index neg_index = grid.getPeriodicIndex(-3, -4);
		REQUIRE(neg_index.row == 1);
		REQUIRE(neg_index.col == 1);
		const Index big_neg_index = grid.getPeriodicIndex(-20, -11);
		REQUIRE(big_neg_index.row == 0);
		REQUIRE(big_neg_index.col == 4);
	}
}
