#include "Util.h"
#include "Point.h"
#include "Matrix.h"
#include "Grid.h"
#include "SuperFill.h"
#include <limits>
#include <cstdint>
#include <functional>
#include <chrono>
#include <set>
#include <boost/optional.hpp>

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

template<typename C>
void RotateOrigins(const Point& edge, const Point& size,
	std::vector<std::pair<Point, C>>& origins)
{
	if (edge.x == -1) {
		for (auto& origin : origins) {
			auto& pos = origin.first;
			if (pos.y == edge.y) {
				pos.x = (pos.x + 1) % size.x;
			}
		}
	} else if (edge.x == size.x) {
		for (auto& origin : origins) {
			auto& pos = origin.first;
			if (pos.y == edge.y) {
				pos.x = (pos.x + size.x - 1) % size.x;
			}
		}
	} else if (edge.y == -1) {
		for (auto& origin : origins) {
			auto& pos = origin.first;
			if (pos.x == edge.x) {
				pos.y = (pos.y + 1) % size.y;
			}
		}
	} else if (edge.y == size.y) {
		for (auto& origin : origins) {
			auto& pos = origin.first;
			if (pos.x == edge.x) {
				pos.y = (pos.y + size.y - 1) % size.y;
			}
		}
	}
}


struct SuperMove {
	SuperMove() = default;
	SuperMove(int x, int y) : move(x, y) {}
	SuperMove(const Point& pos) : move(pos) {}

	operator bool() const {
		return move.x != -1 && move.y != -1;
	}

	Point move;
};


using SuperMatrix = Matrix<SuperMove>;
using SuperOrigin = std::pair<Point, SuperMove>;


int Fitness(Grid& grid, int player, Field extra) {
	auto size = grid.Size();
	int best_fitness = 0;

	for (const auto& v : GetPushVariations(size, extra)) {
		auto field = grid.Push(v.edge, v.tile);
		auto player_pos = grid.Positions()[player];
		Matrix<int> reachable(size.x, size.y, 0);
		FloodFill(grid.Fields(), {{player_pos, 1}}, reachable);

		int display_count = 0;
		int filled_count = 0;
		for (const auto& display_pos : grid.Displays()) {
			if (IsValid(display_pos) && reachable.At(display_pos)) {
				++display_count;
			}
		}

		for (auto x : reachable.GetFields()) {
			filled_count += !!x;
		}

		int current_fitness = display_count + filled_count;
		best_fitness = std::max(best_fitness, current_fitness);
		grid.Push(v.opposite_edge, field);
	}

	return best_fitness;
}

boost::optional<Response> SingleMove(
	Grid& grid, int player, int target, Field extra)
{
	auto size = grid.Size();
	int best_fitness = 0;
	boost::optional<Response> response;

	for (const auto& v : GetPushVariations(size, extra)) {
		auto field = grid.Push(v.edge, v.tile);
		auto player_pos = grid.Positions()[player];
		auto target_pos = grid.Displays()[target];
		Matrix<int> reachable(size.x, size.y, 0);

		FloodFill(grid.Fields(), {{player_pos, 1}}, reachable);
		if (reachable.At(target_pos)) {
			grid.UpdatePosition(player, target_pos);
			grid.UpdateDisplay(target, {});

			auto fitness = Fitness(grid, player, field);
			if (fitness > best_fitness) {
				best_fitness = fitness;
				response = {{v.edge, v.tile}, target_pos};
			}

			grid.UpdateDisplay(target, target_pos);
			grid.UpdatePosition(player, player_pos);
		}

		grid.Push(v.opposite_edge, field);
	}
	return response;
}

int Proximity(const Grid& grid, const Point& p, const Point& q, int penalty) {
	auto size = grid.Size();
	auto dx = std::abs(p.x - q.x);
	auto dy = std::abs(p.y - q.y);

	dx = std::min(dx, size.x - dx);
	dy = std::min(dy, size.y - dy);

	auto dst = dx + dy;

	if ((dx == 0 && dy > 1) || (dy == 0 && dx > 1)) {
		dst += penalty;
	}

	// dst *= 4;
	// dst += 4 - OpenCount(grid.At(p));
	// dst += 4 - OpenCount(grid.At(q));

	return dst;
}

int BasicFitness(const Grid& grid, const Point& pos) {
	auto size = grid.Size();
	Matrix<int> reachable(size.x, size.y, 0);
	FloodFill(grid.Fields(), {{pos, 1}}, reachable);

	int display_count = 0;
	int filled_count = 0;
	for (const auto& display_pos : grid.Displays()) {
		if (IsValid(display_pos) && reachable.At(display_pos)) {
			++display_count;
		}
	}

	for (auto x : reachable.GetFields()) {
		filled_count += !!x;
	}

	return display_count + filled_count;
}

boost::optional<Response> DoubleMove(
	Grid& grid, int player, int target, Field extra)
{
	auto size = grid.Size();
	boost::optional<Response> response;
	int best_fitness = 0;
	int best_distance = std::numeric_limits<int>::max();

	for (const auto& v : GetPushVariations(size, extra)) {
		auto field = grid.Push(v.edge, v.tile);
		auto player_pos = grid.Positions()[player];
		auto target_pos = grid.Displays()[target];
		Matrix<int> reachable(size.x, size.y, 0);
		std::vector<SuperOrigin> origins;
		std::set<Point> move_candidates;

		FloodFill(grid.Fields(), {{player_pos, 1}}, reachable);
		reachable.ForeachField([&](const Point& pos, int& cell) {
			if (cell) {
				origins.emplace_back(pos, pos);
			}
		});

		for (const auto& v2 : GetPushVariations(size, field)) {
			auto field2 = grid.Push(v2.edge, v2.tile);
			auto target_pos2 = grid.Displays()[target];
			Matrix<SuperMove> reachable2(size.x, size.y, {});
			RotateOrigins(v2.edge, size, origins);
			FloodFill(grid.Fields(), origins, reachable2);

			auto cell = reachable2.At(target_pos2);
			if (cell) {
				#if 0
				auto fitness = BasicFitness(grid, target_pos2);
				if (fitness > best_fitness) {
					const auto& move = cell.move;
					auto opt_move = (move == player_pos ? Point{} : move);
					best_fitness = fitness;
					response = {{v.edge, v.tile}, opt_move};
				}
				#else
				move_candidates.insert(cell.move);
				#endif
			}

			RotateOrigins(v2.opposite_edge, size, origins);
			grid.Push(v2.opposite_edge, field2);
		}

		#if 1
		for (const auto& move : move_candidates) {
			grid.UpdatePosition(player, move);

			auto distance = Proximity(grid, move, target_pos, 3);
			if (distance < best_distance) {
				auto opt_move = (move == player_pos ? Point{} : move);
				best_distance = distance;
				response = {{v.edge, v.tile}, opt_move};
			}

			grid.UpdatePosition(player, player_pos);
		}
		#endif

		grid.Push(v.opposite_edge, field);
	}
	return response;
}

int ConvergeDistance(const Grid& grid, const Point& p, const Point& q, int penalty) {
	auto size = grid.Size();
	auto dx = std::abs(p.x - q.x);
	auto dy = std::abs(p.y - q.y);

	dx = std::min(dx, size.x - dx);
	dy = std::min(dy, size.y - dy);
	auto dst = dx + dy;

	if ((dx == 0 && dy > 0) || (dy == 0 && dx > 1)) {
		dst += penalty;
	}

	return dst;
}

boost::optional<Response> ConvergeMove(
	Grid& grid, int player, int target, Field extra)
{
	auto size = grid.Size();
	boost::optional<Response> response;
	int best_distance = std::numeric_limits<int>::max();

	for (const auto& v : GetPushVariations(size, extra)) {
		auto field = grid.Push(v.edge, v.tile);
		auto player_pos = grid.Positions()[player];
		auto target_pos = grid.Displays()[target];
		Matrix<int> reachable(size.x, size.y, 0);

		FloodFill(grid.Fields(), {{player_pos, 1}}, reachable);
		reachable.ForeachField([&](const Point& pos, int& cell) {
			if (cell) {
				auto distance = ConvergeDistance(grid, pos, target_pos, 3);
				if (distance < best_distance) {
					auto opt_move = (pos == player_pos ? Point{} : pos);
					best_distance = distance;
					response = {{v.edge, v.tile}, opt_move};
				}
			}
		});

		grid.Push(v.opposite_edge, field);
	}

	return response;
}

using Clock = std::chrono::steady_clock;

void TimeStat(const std::string& info, Clock::time_point start_t) {
	using MilliSec = std::chrono::milliseconds;
	auto end_t = Clock::now();
	std::cerr << info << " ";
	std::cerr << std::chrono::duration_cast<MilliSec>(end_t - start_t).count();
	std::cerr << " ms" << std::endl;
}

Response SuperFill(Grid grid, int player, int target, Field extra) {
	auto start_t = Clock::now();
	const int max_depth = 2;

	Response response;
	int best = std::numeric_limits<int>::max();
	int closest = std::numeric_limits<int>::max();

	auto player_pos = grid.Positions()[player];
	auto display_pos = grid.Displays()[target];
	auto size = grid.Size();

	auto single_move = SingleMove(grid, player, target, extra);
	if (single_move) {
		TimeStat("SINGLEMOVE", start_t);
		return *single_move;
	}

	auto double_move = DoubleMove(grid, player, target, extra);
	if (double_move) {
		TimeStat("DOUBLEMOVE", start_t);
		return *double_move;
	}

	auto converge_move = ConvergeMove(grid, player, target, extra);
	if (converge_move) {
		TimeStat("CONVERGE", start_t);
		return *converge_move;
	}

	assert(false && "Should not be here");
	return response;
}

} // namespace


void SuperSolver::Turn(const Grid& grid, int player, int target, Field field,
	Callback fn)
{
	Response response = SuperFill(grid, player, target, field);
	fn(response);
}
