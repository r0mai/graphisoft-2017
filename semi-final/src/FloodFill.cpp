#include "FloodFill.h"

#include <stack>
#include <cassert>

#include "Util.h"

Matrix<int> FloodFill(
	const Matrix<Field>& fields,
	const Point& origin)
{
	auto width = fields.Width();
	auto height = fields.Height();

	Matrix<int> reachable{width, height, 0};

	FloodFillTo(reachable, fields, origin, 1);

	return reachable;
}

void FloodFillTo(
	Matrix<int>& fill_matrix,
	const Matrix<Field>& fields,
	const Point& origin,
	int fill_value)
{
	std::vector<Point> origins(1, origin);
	return FloodFillTo(fill_matrix, fields, origins, fill_value);
}

void FloodFillTo(
	Matrix<int>& fill_matrix,
	const Matrix<Field>& fields,
	std::vector<Point> origins,
	int fill_value)
{
	auto& stack = origins;
	auto width = fields.Width();
	auto height = fields.Height();

	while (!stack.empty()) {
		Point p = stack.back();
		stack.pop_back();
		if (fill_matrix.At(p) != 0) {
			continue;
		}
		fill_matrix.At(p) = fill_value;

		if (p.x + 1 < width &&
			IsEastOpen(fields.At(p)) &&
			IsWestOpen(fields.At(p.x + 1, p.y)))
		{
			stack.push_back({p.x + 1, p.y});
		}
		if (p.x - 1 >= 0 &&
			IsWestOpen(fields.At(p)) &&
			IsEastOpen(fields.At(p.x - 1, p.y)))
		{
			stack.push_back({p.x - 1, p.y});
		}
		if (p.y + 1 < height &&
			IsSouthOpen(fields.At(p)) &&
			IsNorthOpen(fields.At(p.x, p.y + 1)))
		{
			stack.push_back({p.x, p.y + 1});
		}
		if (p.y - 1 >= 0 &&
			IsNorthOpen(fields.At(p)) &&
			IsSouthOpen(fields.At(p.x, p.y - 1)))
		{
			stack.push_back({p.x, p.y - 1});
		}
	}
}

void FloodFillExtend(
	Matrix<int>& fill_matrix,
	const Matrix<Field>& fields,
	int fill_value)
{
	std::vector<Point> origins;
	for (int y = 0; y < fields.Height(); ++y) {
		for (int x = 0; x < fields.Width(); ++x) {
			if (fill_matrix.At(x, y) != 0) {
				origins.emplace_back(x, y);
			}
			fill_matrix.At(x, y) = 0;
		}
	}
	FloodFillTo(fill_matrix, fields, origins, fill_value);
}

Matrix<int> FullFloodFill(const Matrix<Field>& fields, int start_index) {
	auto width = fields.Width();
	auto height = fields.Height();

	Matrix<int> fill_map{width, height, 0};

	int color = start_index;
	for (int x = 0; x < fields.Width(); ++x) {
		for (int y = 0; y < fields.Height(); ++y) {
			if (fill_map.At(x, y) == 0) {
				FloodFillTo(fill_map, fields, {x, y}, color++);
			}
		}
	}
	return fill_map;
}

void StupidFloodFillInternal(
	Grid& grid,
	Field extra,
	Matrix<int>& colors,
	std::vector<PushVariation>& pushes,
	int depth,
	int max_depth)
{
	if (depth >= max_depth) {
		return;
	}

	auto bounds = Grow(GetBounds(colors), grid.Size());
	auto varitions = GetPushVariations(bounds, grid.Size(), extra);
	auto colors_copy = colors;

	for (auto& variation : varitions) {
		pushes.push_back(variation);
		auto new_extra = grid.Push(variation.edge, variation.tile);
		colors.Rotate(variation.edge);
		colors_copy.Rotate(variation.edge);

		auto new_colors = colors_copy;
		FloodFillExtend(new_colors, grid.Fields(), depth+1);

		bool changed = false;
		MergeMatrices(colors, new_colors, [&changed](int base, int stuff) {
			int new_value = base;
			if (base == 0) { new_value = stuff; }
			else { new_value = std::min(base, stuff); }
			if (new_value != base) { changed = true; }
			return new_value;
		});
		std::cout << variation.edge << " and " << variation.tile << std::endl;
		std::cout << new_colors << std::endl;

		StupidFloodFillInternal(grid, new_extra, colors, pushes, depth+1, max_depth);

		colors_copy.RotateBack(variation.edge);
		colors.RotateBack(variation.edge);
		grid.Push(variation.opposite_edge, new_extra);
		pushes.pop_back();
	}
}

Matrix<int> StupidFloodFill(Grid grid, const Point& origin, Field extra) {
	auto width = grid.Width();
	auto height = grid.Height();

	Matrix<int> colors(width, height);
	FloodFillTo(colors, grid.Fields(), origin, 1);

	std::vector<PushVariation> pushes;
	StupidFloodFillInternal(grid, extra, colors, pushes, 1, 2);

	return colors;
}
