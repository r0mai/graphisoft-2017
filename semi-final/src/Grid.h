#pragma once
#include <vector>
#include <cstdlib>
#include <cassert>
#include <iostream>

#include "Point.h"
#include "Matrix.h"

class Grid {
public:
    int Width() const { return fields_.Width(); }
    int Height() const { return fields_.Height();; }
    Point Size() const { return {fields_.Width(), fields_.Height()}; }

    void Init(int width, int height, int displays, int players) {
		fields_ = Matrix<int>(width, height, 0);
        display_.resize(displays, {-1, -1});
        position_.resize(players, {-1, -1});
    }

    void Randomize() {
		for (int x = 0; x < fields_.Width(); ++x) {
			for (int y = 0; y < fields_.Height(); ++y) {
				fields_.At(x, y) = 1 + (rand() % 15);
			}
		}
    }

    void UpdateFields(std::vector<int> fields) {
		fields_.SetFields(std::move(fields));
    }

    void UpdateDisplay(int index, const Point& pos) {
        display_[index] = pos;
    }

    void UpdatePosition(int player, const Point& pos) {
        position_[player] = pos;
    }

    int At(int x, int y) const {
		return fields_.At(x, y);
    }


private:
    int players_ = -1;
    std::vector<Point> display_;
    std::vector<Point> position_;
	Matrix<int> fields_;
};

