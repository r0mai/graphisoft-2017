#include "UpwindSailer.h"

#include <boost/optional.hpp>

namespace {

bool areWeAndTargetInSameRow(const Grid& grid, int player, int target) {
	const auto& playerPosition = grid.Positions()[player];
	const auto& displayPosition = grid.Displays()[target];
	return playerPosition.y == displayPosition.y;
}

bool areWeAndTargetInSameColumn(const Grid& grid, int player, int target) {
	const auto& playerPosition = grid.Positions()[player];
	const auto& displayPosition = grid.Displays()[target];
	return playerPosition.x == displayPosition.x;
}

using Direction = Point;

boost::optional<Direction> inchCloser(const Grid& grid, int player, int target)
{
	if (areWeAndTargetInSameRow(grid, player, target)) {
		return boost::none;
	}
	if (areWeAndTargetInSameColumn(grid, player, target)) {
		return boost::none;
	}

	const auto& playerPosition = grid.Positions()[player];
	const auto& displayPosition = grid.Displays()[target];

	auto dx = displayPosition.x - playerPosition.x;
	auto dy = displayPosition.y - playerPosition.y;

	// Prefer large distances to arrive at the diagonal
	if (dx <= dy) {
		return Direction{dx>0?1:-1, 0};
	} else {
		return Direction{0, dy>0?1:-1};
	}
}

boost::optional<Direction> evade(const Grid& grid, int player, int target)
{
	const auto& playerPosition = grid.Positions()[player];
	if (areWeAndTargetInSameRow(grid, player, target)) {
		return Direction{0, playerPosition.x == grid.Height() -1?-1:1};
	}
	if (areWeAndTargetInSameColumn(grid, player, target)) {
		return Direction{playerPosition.y == grid.Width() -1?-1:1, 0};
	}
	return boost::none;
}

Point getEdgeForRelativePush(const Grid& grid,
		int player, const Direction& direction) {
	const auto& position = grid.Positions()[player];
	const auto& dx = direction.x;
	const auto& dy = direction.y;
	assert(std::abs(dx) <=1);
	assert(std::abs(dy) <=1);
	assert(dx * dy == 0);

	auto x = 0;
	auto y = 0;

	switch(dx) {
		case 0:
			x = position.x;
			break;
		case -1:
			x = grid.Width();
			break;
		case  1:
			x = -1;
			break;
		default:
			assert(false);
	}

	switch(dy) {
		case 0:
			y = position.y;
			break;
		case -1:
			y = grid.Height();
			break;
		case  1:
			y = -1;
			break;
		default:
			assert(false);
	}


	return Point{x, y};
}

} // anonymous namespace

void UpwindSailer::Init(int player) {
	this->player = player;
}

void UpwindSailer::Shutdown() {
}

void UpwindSailer::Update(const Grid&, int) {
}

void UpwindSailer::Turn(
		const Grid& grid, int player, int target, Field field, Callback fn) {
	(void)player;
	assert(player == this->player);
	this->grid = grid;
	this->target_display = target;
	boost::optional<Direction> direction = inchCloser(grid, player, target);
	Response response{{{-1, -1}, field}, {-1, -1}};
	if (direction) {
		// TODO: See if we can move closer by using our GOTO
		response.push.edge =
				getEdgeForRelativePush(grid, player, *direction);
	} else {
		// Already on same row/column as target
		boost::optional<Direction> evasion = evade(grid, player, target);
		assert(evasion);
		response.push.edge = getEdgeForRelativePush(grid, player, *evasion);
	}
	fn(response);
}

void UpwindSailer::Idle() {
}
