#include "FloodFill.h"

#include <stack>
#include <cassert>

Matrix<int> FloodFill(
	const Matrix<Field>& fields,
	const Point& origin)
{
	auto width = fields.Width();
	auto height = fields.Height();

	Matrix<int> reachable{width, height, 0};
	Matrix<Direction> direction_matrix{width, height, Direction::None};

	FloodFillTo(reachable, direction_matrix, fields, origin, 1);

	return reachable;
}

void FloodFillTo(
	Matrix<int>& fill_matrix,
	Matrix<Direction>& direction_matrix,
	const Matrix<Field>& fields,
	const Point& origin,
	int fill_value)
{
	std::vector<Point> origins(1, origin);
	FloodFillPoints(fill_matrix, direction_matrix, fields, origins, fill_value);
}

void FloodFillPoints(
	Matrix<int>& fill_matrix,
	Matrix<Direction>& direction_matrix,
	const Matrix<Field>& fields,
	std::vector<Point> origins,
	int fill_value)
{
	auto width = fields.Width();
	auto height = fields.Height();

	while (!origins.empty()) {
		Point p = origins.back();
		origins.pop_back();

		if (p.x + 1 < width &&
			IsEastOpen(fields.At(p)) &&
			IsWestOpen(fields.At(p.x + 1, p.y)))
		{
			auto u = Point{p.x + 1, p.y};
			if (fill_matrix.At(u) == 0) {
				origins.push_back(u);
				direction_matrix.At(u) = Direction::Right;
				fill_matrix.At(u) = fill_value;
			}
		}
		if (p.x - 1 >= 0 &&
			IsWestOpen(fields.At(p)) &&
			IsEastOpen(fields.At(p.x - 1, p.y)))
		{
			auto u = Point{p.x - 1, p.y};
			if (fill_matrix.At(u) == 0) {
				origins.push_back(u);
				direction_matrix.At(u) = Direction::Left;
				fill_matrix.At(u) = fill_value;
			}
		}
		if (p.y + 1 < height &&
			IsSouthOpen(fields.At(p)) &&
			IsNorthOpen(fields.At(p.x, p.y + 1)))
		{
			auto u = Point{p.x, p.y + 1};
			if (fill_matrix.At(u) == 0) {
				origins.push_back(u);
				direction_matrix.At(u) = Direction::Up;
				fill_matrix.At(u) = fill_value;
			}
		}
		if (p.y - 1 >= 0 &&
			IsNorthOpen(fields.At(p)) &&
			IsSouthOpen(fields.At(p.x, p.y - 1)))
		{
			auto u = Point{p.x, p.y - 1};
			if (fill_matrix.At(u) == 0) {
				origins.push_back(u);
				direction_matrix.At(u) = Direction::Down;
				fill_matrix.At(u) = fill_value;
			}
		}
	}
}

void FloodFillExtend(
	Matrix<int>& fill_matrix,
	Matrix<Direction>& direction_matrix,
	const Matrix<Field>& fields,
	int origin_value,
	int fill_value)
{
	std::vector<Point> origins;
	for (int x = 0; x < fields.Width(); ++x) {
		for (int y = 0; y < fields.Height(); ++y) {
			if (fill_matrix.At(x, y) == origin_value) {
				origins.push_back({x, y});
			}
		}
	}
	FloodFillPoints(fill_matrix, direction_matrix, fields, origins, fill_value);
}

Matrix<int> FullFloodFill(const Matrix<Field>& fields, int start_index) {
	auto width = fields.Width();
	auto height = fields.Height();

	Matrix<int> fill_map{width, height, 0};
	Matrix<Direction> direction_matrix{width, height};

	int color = start_index;
	for (int x = 0; x < fields.Width(); ++x) {
		for (int y = 0; y < fields.Height(); ++y) {
			if (fill_map.At(x, y) == 0) {
				FloodFillTo(fill_map, direction_matrix, fields, {x, y}, color++);
			}
		}
	}
	return fill_map;
}

void SuperFloodFillInternal(
	Grid& grid,
	Field extra,
	std::vector<PushVariation>& pushes,
	Matrix<MoveChain>& chain,
	Matrix<int>& fill_matrix,
	Matrix<Direction>& direction_matrix,
	int depth,
	int max_depth)
{
	FloodFillExtend(fill_matrix, direction_matrix, grid.Fields(), depth-1, depth);

	std::cout << fill_matrix << std::endl;

	for (int y = 0; y < grid.Height(); ++y) {
		for (int x = 0; x < grid.Width(); ++x) {
			auto& m = chain.At(x, y);
			if (!m.colored && fill_matrix.At(x, y) != 0) {
				m.colored = true;
				m.pushes = pushes;
			}
		}
	}

	if (depth == max_depth) {
		return;
	}

	for (auto& push_variation : GetPushVariations(grid.Size(), extra)) {
		Point actual_edge = Clamp2(push_variation.edge, {0, 0}, grid.Size());
		Point actual_opposite_edge = Clamp2(push_variation.opposite_edge, {0, 0}, grid.Size());

		Field new_extra = grid.Push(push_variation.edge, extra);
		chain.Push(push_variation.edge, chain.At(actual_opposite_edge));
		fill_matrix.Push(push_variation.edge, fill_matrix.At(actual_opposite_edge));
		direction_matrix.Push(push_variation.edge, direction_matrix.At(actual_opposite_edge));

		pushes.push_back(push_variation);
		SuperFloodFillInternal(
			grid, new_extra, pushes, chain, fill_matrix, direction_matrix, depth+1, max_depth);
		pushes.pop_back();

		direction_matrix.Push(push_variation.opposite_edge, direction_matrix.At(actual_edge));
		fill_matrix.Push(push_variation.opposite_edge, fill_matrix.At(actual_edge));
		chain.Push(push_variation.opposite_edge, chain.At(actual_edge));
		grid.Push(push_variation.opposite_edge, new_extra);
	}
}

Matrix<MoveChain> SuperFloodFill(
	Grid grid,
	const Point& origin,
	Field extra)
{
	Matrix<MoveChain> chain(grid.Width(), grid.Height());
	Matrix<int> fill_matrix(grid.Width(), grid.Height());
	Matrix<Direction> direction_matrix(grid.Width(), grid.Height());
	fill_matrix.At(origin) = 1;

	std::vector<PushVariation> pushes;

	SuperFloodFillInternal(
		grid, extra, pushes, chain, fill_matrix, direction_matrix, 2, 3);

	return chain;
}
