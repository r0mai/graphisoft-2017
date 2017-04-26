#include "EagerTaxicab.h"
#include "Util.h"
#include "FloodFill.h"
#include "UpwindSailer.h"
#include <limits>


void EagerTaxicab::Turn(const Grid& grid, int player, int target, Field field, Callback fn) {
	grid_ = grid;
	player_ = player;
	target_ = target;
	extra_ = field;
	fn(GetResponse());
}

Response EagerTaxicab::GetResponse() {
	int best_distance = std::numeric_limits<int>::max();
	int current_distance = TaxicabDistance(
		grid_.Displays()[target_], grid_.Positions()[player_], grid_.Size());

	Response best_response{};

	for (auto& variation : GetPushVariations(grid_.Size(), extra_)) {
		Field new_extra = grid_.Push(variation.edge, variation.tile);

		int distance;
		Point target_pos;
		std::tie(distance, target_pos) = MoveClosestToTarget();
		if (distance < best_distance) {
			best_distance = distance;
			best_response.push.edge = variation.edge;
			best_response.push.field = variation.tile;
			best_response.move = target_pos;
		}

		grid_.Push(variation.opposite_edge, new_extra);
	}

	if (best_distance < current_distance) {
		return best_response;
	} else {
		return UpwindSailerStep(grid_, player_, target_, extra_);
	}
}

std::tuple<int, Point> EagerTaxicab::MoveClosestToTarget() {
	auto player_pos = grid_.Positions()[player_];
	auto target_pos = grid_.Displays()[target_];
	auto size = grid_.Size();
	auto flood_fill = FloodFill(grid_.Fields(), player_pos);

	int closest_distance = TaxicabDistance(target_pos, player_pos, size);
	Point closest_target; // skip move

	// dumb iteration
	for (int x = 0; x < grid_.Width(); ++x) {
		for (int y = 0; y < grid_.Height(); ++y) {
			int distance = TaxicabDistance(target_pos, {x, y}, size);
			if (flood_fill.At(x, y) && distance < closest_distance) {
				closest_distance = distance;
				closest_target = {x, y};
			}
		}
	}
	return std::make_tuple(closest_distance, closest_target);
}
