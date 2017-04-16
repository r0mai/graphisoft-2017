#include "Grid.h"
#include <cassert>


int Grid::Width() const {
	return fields_.Width();
}

int Grid::Height() const {
	return fields_.Height();
}

Point Grid::Size() const {
	return {fields_.Width(), fields_.Height()};
}

void Grid::Init(int width, int height, int displays, int players) {
	fields_ = Matrix<int>(width, height, 0);
	display_.resize(displays, {-1, -1});
	position_.resize(players, {-1, -1});
}

void Grid::Randomize() {
	for (int x = 0; x < fields_.Width(); ++x) {
		for (int y = 0; y < fields_.Height(); ++y) {
			fields_.At(x, y) = 1 + (rand() % 15);
		}
	}
}

void Grid::UpdateFields(std::vector<int> fields) {
	fields_.SetFields(std::move(fields));
}

void Grid::UpdateDisplay(int index, const Point& pos) {
	display_[index] = pos;
}

void Grid::UpdatePosition(int player, const Point& pos) {
	position_[player] = pos;
}

int Grid::At(int x, int y) const {
	return fields_.At(x, y);
}

int Grid::Push(const Point& pos, int t) {
	auto size = Size();

	if (pos.x == -1) {
		assert(pos.y >= 0 && pos.y < size.y);
		for (int x = size.x; x-- > 1; ) {
			std::swap(fields_.At(x, pos.y), fields_.At(x - 1, pos.y));
		}
		std::swap(fields_.At(0, pos.y), t);
	}

	if (pos.x == size.x) {
		assert(pos.y >= 0 && pos.y < size.y);
		for (int x = 0; x < size.x - 1; ++x) {
			std::swap(fields_.At(x, pos.y), fields_.At(x + 1, pos.y));
		}
		std::swap(fields_.At(size.x - 1, pos.y), t);
	}

	if (pos.y == -1) {
		assert(pos.x >= 0 && pos.x < size.x);
		for (int y = size.y; y-- > 1; ) {
			std::swap(fields_.At(pos.x, y), fields_.At(pos.x, y - 1));
		}
		std::swap(fields_.At(pos.x, 0), t);
	}

	if (pos.y == size.y) {
		assert(pos.x >= 0 && pos.x < size.x);
		for (int y = 0; y < size.y - 1; ++y) {
			std::swap(fields_.At(pos.x, y), fields_.At(pos.x, y + 1));
		}
		std::swap(fields_.At(pos.x, size.y - 1), t);
	}

	return t;
}
