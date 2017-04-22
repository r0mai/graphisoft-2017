#include "UpwindSailer.h"

#include "FloodFill.h"
#include "Util.h"

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

	auto dx = std::abs(displayPosition.x - playerPosition.x);
	auto dy = std::abs(displayPosition.y - playerPosition.y);

	dx = std::min(dx, grid.Width() - dx);
	dy = std::min(dy, grid.Height() - dy);

	if ((playerPosition.x + dx) % grid.Width() != displayPosition.x) {
		dx = -dx;
	}

	if ((playerPosition.y + dy) % grid.Height() != displayPosition.y) {
		dy = -dy;
	}

	// Prefer large distances to arrive at the diagonal
	if (std::abs(dx) >= std::abs(dy)) {
		return Direction{dx>0?1:-1, 0};
	} else {
		return Direction{0, dy>0?1:-1};
	}
}

boost::optional<Direction> evade(const Grid& grid, int player, int target) {
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
	assert(std::abs(dx) <= 1);
	assert(std::abs(dy) <= 1);
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

Response::Push solveEdge(const Grid& grid, int player, int target, Field field) {
	auto playerPos = grid.Positions()[player];
	auto targetPos = grid.Displays()[target];
	auto size = grid.Size();

	if (playerPos.x == targetPos.x) {
		auto x = playerPos.x;
		auto yMin = std::min(playerPos.y, targetPos.y);
		auto yMax = std::max(playerPos.y, targetPos.y);
		auto split = (yMin == 0 && yMax == size.y - 1);

		if (!split) {
			if (yMin == 0) {
				return {Point{x, size.y}, SouthFacing(field)};
			} else {
				return {Point{x, -1}, NorthFacing(field)};
			}
		} else {
			if (IsNorthOpen(grid.At(x, yMin))) {
				return {Point{x, -1}, SouthFacing(field)};
			} else {
				return {Point{x, size.y}, NorthFacing(field)};
			}
		}
	}

	if (playerPos.y == targetPos.y) {
		auto y = playerPos.y;
		auto xMin = std::min(playerPos.x, targetPos.x);
		auto xMax = std::max(playerPos.x, targetPos.x);
		auto split = (xMin == 0 && xMax == size.x - 1);

		if (!split) {
			if (xMin == 0) {
				return {Point{size.x, y}, EastFacing(field)};
			} else {
				return {Point{-1, y}, WestFacing(field)};
			}
		} else {
			if (IsWestOpen(grid.At(xMin, y))) {
				return {Point{-1, y}, EastFacing(field)};
			} else {
				return {Point{size.x, y}, WestFacing(field)};
			}
		}
	}

	assert(false && "Should not be here");
	return {};
}


Response::Push unwrap(const Grid& grid, int player, int target, Field field) {
	assert(grid.IsNeighbor(player, target));
	auto playerPos = grid.Positions()[player];
	auto targetPos = grid.Displays()[target];
	auto size = grid.Size();
	Response::Push push;
	push.field = field;

	if (playerPos.x == targetPos.x) {
		auto backDistance = std::min(playerPos.y, targetPos.y);
		auto forwDistance = size.y - 1 - std::max(playerPos.y, targetPos.y);

		if (backDistance == 0 || forwDistance == 0) {
			push = solveEdge(grid, player, target, field);
		} else {
			push.edge.x = playerPos.x;
			push.edge.y = (forwDistance < backDistance ? -1 : size.y);
		}
	} else if (playerPos.y == targetPos.y) {
		auto backDistance = std::min(playerPos.x, targetPos.x);
		auto forwDistance = size.x - 1 - std::max(playerPos.x, targetPos.x);

		if (backDistance == 0 || forwDistance == 0) {
			push = solveEdge(grid, player, target, field);
		} else {
			push.edge.y = playerPos.y;
			push.edge.x = (forwDistance < backDistance ? -1 : size.x);
		}
	} else {
		assert(false && "Something went wrong");
	}

	return push;
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
		const Grid& newGrid, int player, int target, Field field, Callback fn) {
	auto response = UpwindSailerStep(newGrid, player, target, field);
	fn(response);
}

void UpwindSailer::Idle() {
}


Response UpwindSailerStep(const Grid& newGrid, int player, int target, Field field) {
	auto grid = newGrid;
	Response response;
	response.push.field = field;
	boost::optional<Direction> direction = inchCloser(grid, player, target);

	if (direction) {
		response.push.edge =
				getEdgeForRelativePush(grid, player, *direction);
	} else if (grid.IsNeighbor(player, target)) {
		response.push = unwrap(grid, player, target, field);
	} else {
		// Already on same row/column as target
		boost::optional<Direction> evasion = evade(grid, player, target);
		assert(evasion);
		response.push.edge = getEdgeForRelativePush(grid, player, *evasion);
	}

	grid.Push(response.push.edge, response.push.field);
	const auto& position = grid.Positions()[player];
	const auto& display = grid.Displays()[target];
	if (FloodFill(grid.Fields(), position).At(display)) {
		// Target is reachable
		response.move = display;
	}
	return response;
}
