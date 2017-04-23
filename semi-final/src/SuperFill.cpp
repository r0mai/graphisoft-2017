#include "Util.h"
#include "Point.h"
#include "Matrix.h"
#include "Grid.h"
#include "SuperFill.h"
#include <limits>
#include <cstdint>
#include <functional>
#include <chrono>

namespace {

using FieldMatrix = Matrix<Field>;

template<typename F>
void ForeachNeighbors(const FieldMatrix& matrix, const Point& p, F func) {
	int width = matrix.Width();
	int height = matrix.Height();
	Field field = matrix.At(p);

	if (p.x + 1 < width &&
		IsEastOpen(field) &&
		IsWestOpen(matrix.At(p.x + 1, p.y)))
	{
		func(p.x + 1, p.y);
	}
	if (p.x - 1 >= 0 &&
		IsWestOpen(field) &&
		IsEastOpen(matrix.At(p.x - 1, p.y)))
	{
		func(p.x - 1, p.y);
	}
	if (p.y + 1 < height &&
		IsSouthOpen(field) &&
		IsNorthOpen(matrix.At(p.x, p.y + 1)))
	{
		func(p.x, p.y + 1);
	}
	if (p.y - 1 >= 0 &&
		IsNorthOpen(field) &&
		IsSouthOpen(matrix.At(p.x, p.y - 1)))
	{
		func(p.x, p.y - 1);
	}
}

template<typename C>
void FloodFill(const FieldMatrix& matrix,
	std::vector<std::pair<Point, C>> origins, Matrix<C>& area)
{
	auto& stack = origins;
	assert(matrix.Width() == area.Width());
	assert(matrix.Height() == area.Height());

	while (!stack.empty()) {
		auto top = stack.back();
		stack.pop_back();
		if (bool(area.At(top.first))) {
			continue;
		}
		area.At(top.first) = top.second;
		ForeachNeighbors(matrix, top.first, [&](int x, int y) {
			if (!bool(area.At(x, y))) {
				stack.push_back({{x, y}, top.second});
			}
		});
	}
}

template<typename C, typename F>
std::vector<std::pair<Point, C>> ExtractOrigins(Matrix<C>& area, F func)
{
	std::vector<std::pair<Point, C>> origins;
	area.ForeachField([&func, &origins](const Point& pos, C& cc) {
		if (cc) {
			origins.push_back({pos, func(cc)});
			cc = C();
		}
	});
	return origins;
}



struct SuperMove {
	int depth = 0;
	Point move;

	SuperMove() = default;

	// I don't know why aggregate initialization doesn't take care of this.
	SuperMove(int depth, Point move) : depth(depth), move(std::move(move)) { }

	operator bool() const {
		return depth != 0;
	}
};

using SuperMatrix = Matrix<SuperMove>;
using SuperOrigin = std::pair<Point, SuperMove>;
using InfluenceMatrix = Matrix<int>;
using InfluenceOrigin = std::pair<Point, int>;


void ExtractInfluence(const FieldMatrix& matrix,
	const InfluenceMatrix& influence, int& row_bits, int& col_bits)
{
	auto width = matrix.Width();
	auto height = matrix.Height();

	for (auto y = 0; y < height; ++y) {
		for (auto x = 0; x < width; ++x) {
			if (!influence.At(x, y)) {
				continue;
			}

			auto field = matrix.At(x, y);
			row_bits |= 1 << y;
			col_bits |= 1 << x;

			if (x > 0 && IsWestOpen(field)) {
				col_bits |= 1 << (x - 1);
			}
			if (x < width - 1 && IsEastOpen(field)) {
				col_bits |= 1 << (x + 1);
			}
			if (y > 0 && IsNorthOpen(field)) {
				row_bits |= 1 << (y - 1);
			}
			if (y < height - 1 && IsSouthOpen(field)) {
				row_bits |= 1 << (y + 1);
			}
		}
	}
}

void SuperFillInternal(Grid& grid, SuperMatrix& matrix, int target, Field extra,
	int depth, int max_depth)
{
	SuperMatrix parent_matrix = matrix;
	auto MergeWith = [](const SuperMatrix& base_matrix) {
		auto fn = [&base_matrix](const Point& p, SuperMove& m) {
			const auto& base = base_matrix.At(p);
			if (base && (!m || base.depth < m.depth)) {
				m = base;
			}
		};
		return fn;
	};

	auto size = grid.Size();
	std::vector<InfluenceOrigin> origins;
	InfluenceMatrix influence(size.x, size.y, 0);
	int row_bits = 0;
	int col_bits = 0;

	for (const auto& x : ExtractOrigins(matrix,
		[](const SuperMove& m) { return m; }))
	{
		origins.emplace_back(x.first, 1);
	}
	origins.emplace_back(grid.Displays()[target], 1);

	FloodFill(grid.Fields(), origins, influence);
	ExtractInfluence(grid.Fields(), influence, row_bits, col_bits);

	for (const auto& v : GetPushVariations(row_bits, col_bits, size, extra)) {
		// apply push
		parent_matrix.Rotate(v.edge);
		auto field = grid.Push(v.edge, v.tile);
		auto local_matrix = parent_matrix;

		// collect origins
		auto origins = ExtractOrigins(local_matrix,
			[&depth](const SuperMove& m) {
				return SuperMove{depth, m.move};
			});

		// fill with current color
		FloodFill(grid.Fields(), origins, local_matrix);

		// merge with parent
		local_matrix.ForeachField(MergeWith(parent_matrix));

		if (depth < max_depth) {
			SuperFillInternal(grid, local_matrix, target, field,
				depth + 1, max_depth);
		}

		// revert push
		grid.Push(v.opposite_edge, field);
		parent_matrix.RotateBack(v.edge);

		// merge with output
		local_matrix.RotateBack(v.edge);
		matrix.ForeachField(MergeWith(local_matrix));
	}
}

void DumpMoves(const SuperMatrix& matrix) {
	for (int y = 0; y < matrix.Height(); ++y) {
		for (int x = 0; x < matrix.Width(); ++x) {
			const auto& sm = matrix.At(x, y);
			std::cout << " " << sm.depth;
			// std::cout << ":" << sm.move;
		}
		std::cout << std::endl;
	}
}

Response SuperFill(Grid grid, int player, int target, Field extra) {
	using Clock = std::chrono::steady_clock;
	using Duration = std::chrono::duration<double>;
	auto start_t = Clock::now();
	const int max_depth = 2;

	Response response;
	int best = std::numeric_limits<int>::max();
	int closest = std::numeric_limits<int>::max();

	auto player_pos = grid.Positions()[player];
	auto display_pos = grid.Displays()[target];
	auto size = grid.Size();

	InfluenceMatrix influence(size.x, size.y, 0);
	int row_bits = 0;
	int col_bits = 0;

	// influence
	FloodFill(grid.Fields(), {{player_pos, 1}, {display_pos, 1}}, influence);
	ExtractInfluence(grid.Fields(), influence, row_bits, col_bits);

	for (const auto& v : GetPushVariations(row_bits, col_bits, size, extra)) {
		auto field = grid.Push(v.edge, v.tile);
		SuperOrigin origin;
		SuperMatrix matrix(size.x, size.y);

		player_pos = grid.Positions()[player];
		display_pos = grid.Displays()[target];

		origin.first = player_pos;
		origin.second = {1, {}};
		FloodFill(grid.Fields(), {origin}, matrix);

		// fix move info
		matrix.ForeachField([](const Point& p, SuperMove& m) {
			if (m) { m.move = p; }
		});
		matrix.At(player_pos).move = {}; // means skip

		SuperFillInternal(grid, matrix, target, field, 2, max_depth);

		const auto& sm = matrix.At(display_pos);
		if (sm && sm.depth < best) {
			response.push.edge = v.edge;
			response.push.field = v.tile;
			response.move = sm.move;
			best = sm.depth;
		}

		if (best > max_depth) {
			matrix.ForeachField([&](const Point& p, SuperMove& m) {
				if (!m || m.depth != 1) {
					return;
				}
				auto dst = TaxicabDistance(p, display_pos, size);
				if (dst < closest) {
					response.push.edge = v.edge;
					response.push.field = v.tile;
					response.move = m.move;
					closest = dst;
				}
			});
		}

		grid.Push(v.opposite_edge, field);

		if (best == 1) {
			break;
		}
	}

	auto end_t = Clock::now();
	std::cerr
		<< "Superfill took "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(end_t - start_t).count()
		<< " ms" << std::endl;

	return response;
}

} // namespace


void SuperSolver::Turn(const Grid& grid, int player, int target, Field field,
	Callback fn)
{
	Response response = SuperFill(grid, player, target, field);
	fn(response);
}
