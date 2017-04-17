#include "EagerTaxicab.h"
#include "Util.h"
#include "FloodFill.h"

#include <limits>


void EagerTaxicab::Turn(const Grid& grid, int player, int target, Field field, Callback fn) {
	grid_ = grid;
	player_ = player;
	target_display_ = target;
	extra_ = field;
	fn(GetResponse());
}

Response EagerTaxicab::GetResponse() {
	int best_distance = std::numeric_limits<int>::max();
	Response best_response{};

	for (auto& variation : GetPushVariations(grid_.Size(), extra_)) {
		Field new_extra = grid_.Push(variation.edge, variation.tile);

		int distance;
		Point target;
		std::tie(distance, target) = MoveClosestToTarget();
		if (distance < best_distance) {
			best_distance = distance;
			best_response.push.edge = variation.edge;
			best_response.push.field = variation.tile;
			best_response.move = target;
		}

		grid_.Push(variation.opposite_edge, new_extra);
	}

	std::cout << "Push direction " << best_response.push.edge << std::endl;

	return best_response;
}

std::tuple<int, Point> EagerTaxicab::MoveClosestToTarget() {
	auto flood_fill = FloodFill(grid_.Fields(), grid_.Positions()[player_]);

	Point target = grid_.Displays()[target_display_];

	int closest_distance = std::numeric_limits<int>::max();
	Point closest_target;

	// dumb iteration
	for (int x = 0; x < grid_.Width(); ++x) {
		for (int y = 0; y < grid_.Height(); ++y) {
			int distance = TaxicabDistance(target, {x, y});
			if (flood_fill.At(x, y) && distance < closest_distance) {
				closest_distance = distance;
				closest_target = {x, y};
			}
		}
	}
	return std::make_tuple(closest_distance, closest_target);
}
