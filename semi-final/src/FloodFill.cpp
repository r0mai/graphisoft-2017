#include "FloodFill.h"

#include <stack>
#include <cassert>
#include <chrono>

#include "Util.h"

Matrix<int> FloodFill(
	const Matrix<Field>& fields,
	const Point& origin)
{
	auto width = fields.Width();
	auto height = fields.Height();

	Matrix<int> reachable{width, height, 0};
	Matrix<Point> parents(width, height);

	FloodFillTo(reachable, parents, fields, origin, 1);

	return reachable;
}

void FloodFillTo(
	Matrix<int>& fill_matrix,
	Matrix<Point>& parents,
	const Matrix<Field>& fields,
	const Point& origin,
	int fill_value)
{
	std::vector<Point> origins(1, origin);
	return FloodFillTo(fill_matrix, parents, fields, origins, fill_value);
}

void FloodFillTo(
	Matrix<int>& fill_matrix,
	Matrix<Point>& parents,
	const Matrix<Field>& fields,
	std::vector<Point> origins,
	int fill_value)
{
	auto& stack = origins;
	auto width = fields.Width();
	auto height = fields.Height();

	std::vector<Point> parents_stack(origins.size());
	for (int i = 0; i < parents_stack.size(); ++i) {
		// origins are their own parents
		parents_stack[i] = origins[i];
	}

	while (!stack.empty()) {
		Point p = stack.back();
		Point parent = parents_stack.back();
		stack.pop_back();
		parents_stack.pop_back();

		if (fill_matrix.At(p) != 0) {
			continue;
		}

		fill_matrix.At(p) = fill_value;
		parents.At(p) = parent;

		if (p.x + 1 < width &&
			IsEastOpen(fields.At(p)) &&
			IsWestOpen(fields.At(p.x + 1, p.y)))
		{
			stack.push_back({p.x + 1, p.y});
			parents_stack.push_back(p);
		}
		if (p.x - 1 >= 0 &&
			IsWestOpen(fields.At(p)) &&
			IsEastOpen(fields.At(p.x - 1, p.y)))
		{
			stack.push_back({p.x - 1, p.y});
			parents_stack.push_back(p);
		}
		if (p.y + 1 < height &&
			IsSouthOpen(fields.At(p)) &&
			IsNorthOpen(fields.At(p.x, p.y + 1)))
		{
			stack.push_back({p.x, p.y + 1});
			parents_stack.push_back(p);
		}
		if (p.y - 1 >= 0 &&
			IsNorthOpen(fields.At(p)) &&
			IsSouthOpen(fields.At(p.x, p.y - 1)))
		{
			stack.push_back({p.x, p.y - 1});
			parents_stack.push_back(p);
		}
	}
}

void FloodFillExtend(
	Matrix<int>& fill_matrix,
	Matrix<Point>& parents,
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
	FloodFillTo(fill_matrix, parents, fields, origins, fill_value);
}

Matrix<int> FullFloodFill(const Matrix<Field>& fields, int start_index) {
	auto width = fields.Width();
	auto height = fields.Height();

	Matrix<int> fill_map{width, height, 0};

	int color = start_index;
	for (int x = 0; x < fields.Width(); ++x) {
		for (int y = 0; y < fields.Height(); ++y) {
			if (fill_map.At(x, y) == 0) {
				Matrix<Point> parents(width, height);
				FloodFillTo(fill_map, parents, fields, {x, y}, color++);
			}
		}
	}
	return fill_map;
}

void StupidFloodFillInternal(
	Grid& grid,
	Field extra,
	Matrix<int>& colors,
	Matrix<Point>& parents,
	Matrix<std::vector<PushVariation>>& push_matrix,
	std::vector<PushVariation>& pushes,
	int depth,
	int max_depth)
{

	if (depth >= max_depth) {
		return;
	}

	auto bounds = Grow(GetBounds(colors), grid.Size());
	auto varitions = GetPushVariations(bounds, grid.Size(), extra);
	auto original_colors = colors;
	auto original_parents = parents;
	auto original_push_matrix = push_matrix;

	// I wrote waay too much javascript lately
	auto fn = [&](
		Matrix<int>& lhs_colors,
		const Matrix<int>& rhs_colors,
		Matrix<Point>& lhs_parents,
		const Matrix<Point>& rhs_parents,
		Matrix<std::vector<PushVariation>>& lhs_push_matrix,
		const Matrix<std::vector<PushVariation>>& rhs_push_matrix)
	{
		return [&](const Point& p) {
			int& base = lhs_colors.At(p);
			int stuff = rhs_colors.At(p);
			int new_value;
			if (base == 0) { new_value = stuff; }
			else if (stuff == 0) { new_value = base; }
			else { new_value = std::min(base, stuff); }
			if (base != new_value) {
				base = new_value;
				assert(IsValid(rhs_parents.At(p)));
				lhs_parents.At(p) = rhs_parents.At(p);
				lhs_push_matrix.At(p) = pushes;
			}
		};
	};

	for (auto& variation : varitions) {
		pushes.push_back(variation);
		auto new_extra = grid.Push(variation.edge, variation.tile);
		auto local_colors = original_colors;
		auto current_colors = original_colors;
		auto local_parents = original_parents;
		auto current_parents = original_parents;
		auto local_push_matrix = original_push_matrix;
		auto current_push_matrix = original_push_matrix;
		local_colors.Rotate(variation.edge);
		current_colors.Rotate(variation.edge);
		local_parents.Rotate(variation.edge);
		current_parents.Rotate(variation.edge);
		local_push_matrix.Rotate(variation.edge);
		current_push_matrix.Rotate(variation.edge);
		ShiftMatrixCoordinates(current_parents, variation.edge);
		ShiftMatrixCoordinates(local_parents, variation.edge);

		FloodFillExtend(current_colors, current_parents, grid.Fields(), depth+1);

		// MergeMatrices(local_colors, current_colors, fn);
		ForEachPoint(grid.Size(), fn(
			local_colors, current_colors,
			local_parents, current_parents,
			local_push_matrix, current_push_matrix));

		StupidFloodFillInternal(
			grid, new_extra, local_colors,
			local_parents, local_push_matrix, pushes, depth+1, max_depth);

		local_colors.RotateBack(variation.edge);
		local_parents.RotateBack(variation.edge);
		local_push_matrix.RotateBack(variation.edge);
		ShiftMatrixCoordinates(local_parents, variation.opposite_edge);
		// MergeMatrices(colors, local_colors, fn);
		ForEachPoint(grid.Size(), fn(
			colors, local_colors,
			parents, local_parents,
			push_matrix, local_push_matrix));

		grid.Push(variation.opposite_edge, new_extra);
		pushes.pop_back();
	}
}

Matrix<int> StupidFloodFill(
	Matrix<Point>& parents,
	Matrix<std::vector<PushVariation>>& push_matrix,
	Grid grid,
	const Point& origin,
	Field extra)
{
	using Clock = std::chrono::steady_clock;
	using Duration = std::chrono::duration<double>;
	auto width = grid.Width();
	auto height = grid.Height();

	auto start_t = Clock::now();
	Matrix<int> colors(width, height);
	FloodFillTo(colors, parents, grid.Fields(), origin, 1);

	std::vector<PushVariation> pushes;
	StupidFloodFillInternal(grid, extra, colors, parents, push_matrix, pushes, 1, 3);
	auto end_t = Clock::now();

	std::cout << parents << std::endl;

	std::cerr
		<< "Delta T: "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(end_t - start_t).count()
		<< " ms" << std::endl;

	return colors;
}

void ShiftMatrixCoordinates(Matrix<Point>& parents, const Point& edge) {
	int w = parents.Width();
	int h = parents.Height();
	if (edge.x == -1) {
		assert(edge.y >= 0 && edge.y < h);
		ForEachPoint({w, h}, [&](const Point& p) {
			auto& parent = parents.At(p);
			if (parent.y == edge.y) {
				++parent.x; if (parent.x == w) { parent.x = 0; }
			}
		});
	}

	if (edge.x == w) {
		assert(edge.y >= 0 && edge.y < h);
		ForEachPoint({w, h}, [&](const Point& p) {
			auto& parent = parents.At(p);
			if (parent.y == edge.y) {
				--parent.x; if (parent.x == -1) { parent.x = w-1; }
			}
		});
	}

	if (edge.y == -1) {
		ForEachPoint({w, h}, [&](const Point& p) {
			auto& parent = parents.At(p);
			if (parent.x == edge.x) {
				++parent.y; if (parent.y == h) { parent.y = 0; }
			}
		});
	}

	if (edge.y == h) {
		ForEachPoint({w, h}, [&](const Point& p) {
			auto& parent = parents.At(p);
			if (parent.x == edge.x) {
				--parent.y; if (parent.y == -1) { parent.y = h-1; }
			}
		});
	}
}
