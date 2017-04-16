#pragma once
#include <vector>
#include <cstdlib>
#include <cassert>
#include <iostream>

#include "Point.h"

class Grid {
public:
    int Width() const { return width_; }
    int Height() const { return height_; }
    Point Size() const { return {width_, height_}; }

    void Init(int width, int height, int displays, int players) {
        width_ = width;
        height_ = height;
        display_.resize(displays, {-1, -1});
        position_.resize(players, {-1, -1});
        fields_.resize(width * height, 0);
    }

    void Randomize() {
        for (auto& v : fields_) {
            v = 1 + (rand() % 15);
        }
    }

    void UpdateFields(std::vector<int> fields) {
		assert(fields.size() == width_ * height_);
        fields_ = std::move(fields);
    }

    void UpdateDisplay(int index, const Point& pos) {
        display_[index] = pos;
    }

    void UpdatePosition(int player, const Point& pos) {
        position_[player] = pos;
    }

    int At(int x, int y) const {
        assert(x >= 0 && y >= 0 && x < width_ && y < height_);
        return fields_[x + y * width_];
    }


private:
    int width_ = -1;
    int height_ = -1;
    int players_ = -1;
    std::vector<int> fields_;
    std::vector<Point> display_;
    std::vector<Point> position_;
};

