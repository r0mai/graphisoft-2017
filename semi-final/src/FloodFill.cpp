#include "FloodFill.h"

#include <stack>
#include <cassert>
#include <chrono>

#include "Util.h"

namespace {

template<typename F>
void MergeMatrices(Matrix<int>& base, const Matrix<int>& other, F fn) {
	for (int y = 0; y < base.Height(); ++y) {
		for (int x = 0; x < base.Width(); ++x) {
			base.At(x, y) = fn(base.At(x, y), other.At(x, y));
		}
	}
}


} // namespace

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

void StupidFloodFillInternal(
	Grid& grid,
	Field extra,
	Matrix<int>& colors,
	std::vector<PushVariation>& pushes,
	int depth,
	int max_depth)
{
	auto fn = [](int base, int stuff) {
		int new_value = base;
		if (base == 0) { new_value = stuff; }
		else if (stuff == 0) { new_value = base; }
		else { new_value = std::min(base, stuff); }
		return new_value;
	};

	if (depth > max_depth) {
		return;
	}

	auto bounds = Grow(GetBounds(colors), grid.Size());
	auto varitions = GetPushVariations(grid.Size(), extra);
	auto original_colors = colors;

	for (auto& variation : varitions) {
		pushes.push_back(variation);
		auto new_extra = grid.Push(variation.edge, variation.tile);
		auto local_colors = original_colors;
		auto current_colors = original_colors;
		local_colors.Rotate(variation.edge);
		current_colors.Rotate(variation.edge);

		FloodFillExtend(current_colors, grid.Fields(), depth);

		MergeMatrices(local_colors, current_colors, fn);

		StupidFloodFillInternal(grid, new_extra, local_colors, pushes, depth + 1, max_depth);

		local_colors.RotateBack(variation.edge);
		MergeMatrices(colors, local_colors, fn);
		grid.Push(variation.opposite_edge, new_extra);
		pushes.pop_back();
	}
}

Matrix<int> StupidFloodFill(Grid grid, const Point& origin, Field extra, bool move_first) {
	using Clock = std::chrono::steady_clock;
	using Duration = std::chrono::duration<double>;
	auto width = grid.Width();
	auto height = grid.Height();

	auto start_t = Clock::now();
	Matrix<int> colors(width, height, 0);
	if (move_first) {
		FloodFillTo(colors, grid.Fields(), origin, 1);
	} else {
		colors.At(origin) = 1;
	}

	std::vector<PushVariation> pushes;
	StupidFloodFillInternal(grid, extra, colors, pushes, 2, 3);
	auto end_t = Clock::now();

	std::cerr
		<< "COLORING "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(end_t - start_t).count()
		<< " ms" << (move_first ? " [move]" : " [push]") << std::endl;

	return colors;
}
