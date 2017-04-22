#include "Util.h"
#include "Point.h"
#include "Matrix.h"
#include "Grid.h"
#include "SuperFill.h"
#include <limits>


namespace {

using FieldMatrix = Matrix<Field>;
void ForeachNeighbors(const FieldMatrix& matrix, const Point& p,
	std::function<void(const Point&)> func)
{
	int width = matrix.Width();
	int height = matrix.Height();

	if (p.x + 1 < width &&
		IsEastOpen(matrix.At(p)) &&
		IsWestOpen(matrix.At(p.x + 1, p.y)))
	{
		func({p.x + 1, p.y});
	}
	if (p.x - 1 >= 0 &&
		IsWestOpen(matrix.At(p)) &&
		IsEastOpen(matrix.At(p.x - 1, p.y)))
	{
		func({p.x - 1, p.y});
	}
	if (p.y + 1 < height &&
		IsSouthOpen(matrix.At(p)) &&
		IsNorthOpen(matrix.At(p.x, p.y + 1)))
	{
		func({p.x, p.y + 1});
	}
	if (p.y - 1 >= 0 &&
		IsNorthOpen(matrix.At(p)) &&
		IsSouthOpen(matrix.At(p.x, p.y - 1)))
	{
		func({p.x, p.y - 1});
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
		ForeachNeighbors(matrix, top.first, [&](const Point& p) {
			stack.push_back({p, top.second});
		});
	}
}

} // namespace

struct SuperMove {
	int depth = 0;
	Point move;

	operator bool() const {
		return depth != 0;
	}
};

using SuperMatrix = Matrix<SuperMove>;
using SuperOrigin = std::pair<Point, SuperMove>;

void SuperFillInternal(Grid& grid, SuperMatrix& matrix, Field extra,
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

	for (const auto& v : GetPushVariations(grid.Size(), extra)) {
		// apply push
		parent_matrix.Rotate(v.edge);
		auto field = grid.Push(v.edge, v.tile);
		auto local_matrix = parent_matrix;
		std::vector<SuperOrigin> origins;

		// collect origins
		local_matrix.ForeachField([&](const Point& p, SuperMove& m) {
			if (m) {
				origins.push_back({p, {depth, m.move}});
				m.depth = 0;
			}
		});

		// fill with current color
		FloodFill(grid.Fields(), origins, local_matrix);

		// merge with parent
		local_matrix.ForeachField(MergeWith(parent_matrix));

		if (depth < max_depth) {
			SuperFillInternal(grid, local_matrix, field, depth + 1, max_depth);
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
			std::cout
				<< " "
				<< sm.depth
				// << ":"
				// << sm.move
			;
		}
		std::cout << std::endl;
	}
}

Response SuperFill(Grid grid, int player, int target, Field extra) {
	Response response;
	int best = std::numeric_limits<int>::max();
	for (const auto& v : GetPushVariations(grid.Size(), extra)) {
		auto field = grid.Push(v.edge, v.tile);
		SuperOrigin origin;
		SuperMatrix matrix(grid.Width(), grid.Height());

		auto player_pos = grid.Positions()[player];
		auto display_pos = grid.Displays()[target];
		origin.first = player_pos;
		origin.second = {1, {}};
		FloodFill(grid.Fields(), {origin}, matrix);
		matrix.ForeachField([](const Point& p, SuperMove& m) {
			if (m) {
				m.move = p;
			}
		});
		matrix.At(player_pos).move = {}; // means skip
		SuperFillInternal(grid, matrix, field, 2, 2);

		const auto& sm = matrix.At(display_pos);
		if (sm && sm.depth < best) {
			response.push.edge = v.edge;
			response.push.field = v.tile;
			response.move = sm.move;
			best = sm.depth;
		}
		grid.Push(v.opposite_edge, field);
	}
	return response;
}

void SuperSolver::Turn(const Grid& grid, int player, int target, Field field,
	Callback fn)
{
	Response response = SuperFill(grid, player, target, field);
	fn(response);
}
