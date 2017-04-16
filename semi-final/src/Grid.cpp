#include "Grid.h"
#include <cassert>

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

Point Grid::Size() const {
	return {fields_.Width(), fields_.Height()};
}

const std::vector<Point>& Grid::Displays() const {
	return displays_;
}

void Grid::Init(int width, int height, int displays, int players) {
	fields_ = Matrix<Field>(width, height, Field(0));
	displays_.resize(displays, {-1, -1});
	positions_.resize(players, {-1, -1});
}

void Grid::Randomize() {
	for (int x = 0; x < fields_.Width(); ++x) {
		for (int y = 0; y < fields_.Height(); ++y) {
			fields_.At(x, y) = Field(1 + (rand() % 15));
		}
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

