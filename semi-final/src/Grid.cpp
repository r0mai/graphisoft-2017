#include "Grid.h"
#include "Util.h"
#include <cassert>
#include <set>
#include <algorithm>

namespace {

void ShiftCol(const Point& size, const Point& pos, int d, std::vector<Point>& vec) {
	for (auto& p : vec) {
		if (pos.x == p.x) {
			p.y = (p.y + d + size.y) % size.y;
		}
	}
}

void ShiftRow(const Point& size, const Point& pos, int d, std::vector<Point>& vec) {
	for (auto& p : vec) {
		if (pos.y == p.y) {
			p.x = (p.x + d + size.x) % size.x;
		}
	}
}

std::vector<Point> RandomPositions(int n, int w, int h) {
	assert(n < w * h);
	std::set<int> ps;
	std::vector<Point> result;

	while (ps.size() < n) {
		auto num = rand() % (w * h);
		if (ps.count(num) == 0) {
			result.push_back(Point{num / h, num % h});
		}
		ps.insert(num);
	}
	return result;
}

} // namespace


int Grid::Width() const {
	return fields_.Width();
}

int Grid::Height() const {
	return fields_.Height();
}

int Grid::DisplayCount() const {
	return displays_.size();
}

int Grid::ActiveDisplayCount() const {
	int sum = 0;
	for (auto x : displays_) {
		if (IsValid(x)) {
			++sum;
		}
	}
	return sum;
}

int Grid::PlayerCount() const {
	// note: positions_ might be initalized with more players than the
	// actual player count
	int sum = 0;
	for (auto x : positions_) {
		if (IsValid(x)) {
			++sum;
		}
	}
	return sum;
}

Point Grid::Size() const {
	return {fields_.Width(), fields_.Height()};
}

const std::vector<Point>& Grid::Displays() const {
	return displays_;
}

bool Grid::IsNeighbor(int player, int display) const {
	if (!IsValid(displays_[display])) {
		return false;
	}

	auto p = positions_[player];
	auto q = displays_[display];
	auto dx = std::abs(p.x - q.x);
	auto dy = std::abs(p.y - q.y);

	return
		(dx == 0 && (dy == 1 || dy == Height() - 1)) ||
		(dy == 0 && (dx == 1 || dx == Width() - 1));
}

void Grid::Init(int width, int height, int displays, int players) {
	fields_ = Matrix<Field>(width, height, Field(0));
	displays_.resize(displays, {-1, -1});
	positions_.resize(players, {-1, -1});
}

void Grid::Randomize() {
	int fx[] = {1, 3, 3, 3, 1};		// frequencies of different tiles
	int kx[] = {1, 3, 7, 5, 15};	// tiles

	int ftotal = 0;
	for (auto f : fx) {
		ftotal += f;
	}

	auto width = fields_.Width();
	auto height = fields_.Height();
	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height; ++y) {
			// random tile weighted w/ frequency
			int rnd = rand() % ftotal;
			int k = 0;
			for (auto f : fx) {
				if (rnd < f) { break; }
				rnd -= f;
				++k;
			}

			// random rotate
			Field cc = Field(kx[k]);
			for (int i = rand() % 4; i-- > 0;) {
				cc = RotateLeft(cc);
			}

			fields_.At(x, y) = cc;
		}
	}

	auto k = displays_.size();
	auto l = positions_.size();
	auto ps = RandomPositions(k + l, width, height);

	for (int i = 0; i < k; ++i) {
		displays_[i] = ps[i];
	}
	for (int i = 0; i < l; ++i) {
		positions_[i] = ps[k + i];
	}
}

void Grid::ResetDisplays() {
	for (auto& p : displays_) {
		p.x = p.y = -1;
	}
}

void Grid::UpdateFields(std::vector<Field> fields) {
	fields_.SetFields(std::move(fields));
}

void Grid::UpdateDisplay(int index, const Point& pos) {
	displays_[index] = pos;
}

void Grid::UpdatePosition(int player, const Point& pos) {
	positions_[player] = pos;
}

Field Grid::At(int x, int y) const {
	return fields_.At(x, y);
}

const Matrix<Field>& Grid::Fields() const {
	return fields_;
}

const std::vector<Point>& Grid::Positions() const {
	return positions_;
}

Field Grid::Push(const Point& pos, Field t) {
	auto size = Size();

	if (pos.x == -1) {
		assert(pos.y >= 0 && pos.y < size.y);
		for (int x = size.x; x-- > 1; ) {
			std::swap(fields_.At(x, pos.y), fields_.At(x - 1, pos.y));
		}
		std::swap(fields_.At(0, pos.y), t);
		ShiftRow(size, pos, 1, positions_);
		ShiftRow(size, pos, 1, displays_);
	}

	if (pos.x == size.x) {
		assert(pos.y >= 0 && pos.y < size.y);
		for (int x = 0; x < size.x - 1; ++x) {
			std::swap(fields_.At(x, pos.y), fields_.At(x + 1, pos.y));
		}
		std::swap(fields_.At(size.x - 1, pos.y), t);
		ShiftRow(size, pos, -1, positions_);
		ShiftRow(size, pos, -1, displays_);
	}

	if (pos.y == -1) {
		assert(pos.x >= 0 && pos.x < size.x);
		for (int y = size.y; y-- > 1; ) {
			std::swap(fields_.At(pos.x, y), fields_.At(pos.x, y - 1));
		}
		std::swap(fields_.At(pos.x, 0), t);
		ShiftCol(size, pos, 1, positions_);
		ShiftCol(size, pos, 1, displays_);
	}

	if (pos.y == size.y) {
		assert(pos.x >= 0 && pos.x < size.x);
		for (int y = 0; y < size.y - 1; ++y) {
			std::swap(fields_.At(pos.x, y), fields_.At(pos.x, y + 1));
		}
		std::swap(fields_.At(pos.x, size.y - 1), t);
		ShiftCol(size, pos, -1, positions_);
		ShiftCol(size, pos, -1, displays_);
	}

	return t;
}

Field Grid::Push(int c, int p, int k, Field t) {
	auto size = Size();
	if (c == 0) {
		return Push({p == 0 ? size.x : -1, k}, t);
	} else {
		return Push({k, p == 0 ? size.y : -1}, t);
	}
}

Grid::Delta Grid::Diff(const Grid& grid, Field extra, int player) const {
	auto size = Size();
	std::vector<std::pair<Point, Field>> candidates;
	extra = Normalize(extra);
	Field tile;

	for (int x = 0; x < size.x; ++x) {
		if (Normalize(tile = At(x, 0)) == extra) {
			candidates.push_back({{x, -1}, tile});
		}
		if (Normalize(tile = At(x, size.y - 1)) == extra) {
			candidates.push_back({{x, size.y}, tile});
		}
	}

	for (int y = 0; y < size.y; ++y) {
		if (Normalize(tile = At(0, y)) == extra) {
			candidates.push_back({{-1, y}, tile});
		}
		if (Normalize(tile = At(size.x - 1, y)) == extra) {
			candidates.push_back({{size.x, y}, tile});
		}
	}


	for (const auto& cc : candidates) {
		auto copy = grid;
		tile = copy.Push(cc.first, cc.second);
		if (copy.fields_ == fields_) {
			int disappeared = 0;
			int last_disappeared = -1;
			bool mismatch = false;
			for (int i = 0, ie = displays_.size(); i < ie; ++i) {
				auto dpos = displays_[i];
				auto cpos = copy.displays_[i];
				if (!IsValid(cpos)) {
					assert(!IsValid(dpos));
					continue;
				}
				if (IsValid(dpos) && cpos != dpos) {
					mismatch = true;
					break;
				}
				if (!IsValid(dpos)) {
					++disappeared;
					last_disappeared = i;
				}
			}

			assert(disappeared <= 1);
			if (mismatch) {
				continue;
			}

			int moved = 0;
			int last_moved = -1;
			for (int i = 0, ie = positions_.size(); i < ie; ++i) {
				if (positions_[i] != copy.positions_[i]) {
					++moved;
					last_moved = i;
				}
			}
			if (moved > 1 || (moved == 1 && last_moved != player)) {
				continue;
			}

			if (last_disappeared >= 0) {
				assert(positions_[player] == copy.displays_[last_disappeared]);
			}

			Delta delta;
			delta.edge = cc.first;
			delta.extra = cc.second;
			delta.scored = disappeared > 0;
			if (moved == 1) {
				delta.move = positions_[player];
			}
			return delta;
		}
	}

	assert(false && "Grids could not move to each other");
	return {};
}

Field Grid::TileDiff(const Grid& grid, Field extra) const {
	std::vector<int> tiles(16);
	for (auto t : grid.fields_.GetFields()) {
		++tiles[Normalize(t)];
	}
	++tiles[Normalize(extra)];
	for (auto t : fields_.GetFields()) {
		--tiles[Normalize(t)];
	}

	int nonzero = 0;
	int missing = -1;
	int tile = -1;
	for (auto x : tiles) {
		++tile;
		if (x != 0) {
			assert(x == 1);
			++nonzero;
			missing = tile;
		}
	}
	assert(nonzero == 1);
	return Field(missing);
}

bool Grid::ScoreDiff(const Grid& grid) const {
	for (int i = 0, ie = displays_.size(); i < ie; ++i) {
		if (IsValid(grid.displays_[i]) && !IsValid(displays_[i])) {
			return true;
		}
	}
	return false;
}



namespace {

struct ConsoleChar {
	std::string ch;
};

} // anonymous namespace

std::ostream& operator<<(std::ostream& os, const Grid& grid) {
	// Full block character didn't work,
	// so print a space with inverted video mode
	// http://stackoverflow.com/a/13437275
	std::string fb = "\x1b[7m \x1b[0m";
	Matrix<ConsoleChar> chars(3*grid.Width(), 3*grid.Height());

	for (int x = 0; x < grid.Width(); ++x) {
		for (int y = 0; y < grid.Height(); ++y) {
			Field f = grid.At(x, y);
			// corners
			chars.At(3*x + 0, 3*y + 0).ch = fb;
			chars.At(3*x + 2, 3*y + 0).ch = fb;
			chars.At(3*x + 0, 3*y + 2).ch = fb;
			chars.At(3*x + 2, 3*y + 2).ch = fb;
			// center
			chars.At(3*x + 1, 3*y + 1).ch = " ";
			// sides
			chars.At(3*x + 1, 3*y + 0).ch = IsNorthOpen(f) ? " " : fb;
			chars.At(3*x + 2, 3*y + 1).ch = IsEastOpen(f) ? " " : fb;
			chars.At(3*x + 1, 3*y + 2).ch = IsSouthOpen(f) ? " " : fb;
			chars.At(3*x + 0, 3*y + 1).ch = IsWestOpen(f) ? " " : fb;
		}
	}
	for (auto& display : grid.Displays()) {
		if (IsValid(display)) {
			chars.At(3*display.x + 1, 3*display.y + 1).ch = "D";
		}
	}
	for (auto& player : grid.Positions()) {
		chars.At(3*player.x + 1, 3*player.y + 1).ch = "P";
	}
	for (int y = 0; y < chars.Height(); ++y) {
		for (int x = 0; x < chars.Width(); ++x) {
			os << chars.At(x, y).ch;
		}
		os << '\n';
	}
	return os;
}
