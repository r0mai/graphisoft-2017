#pragma once
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

    int Extra() const {
        return extra_;
    }

    void RotateLeft() {
        extra_ = (extra_ >> 3) + ((extra_ << 1) & 0xf);
    }

    void RotateRight() {
        extra_ = ((extra_ & 1) << 3) + (extra_ >> 1);
        std::cerr << extra_ << std::endl;
    }


private:
    int width_ = -1;
    int height_ = -1;
    int players_ = -1;
    int extra_ = 3;
    std::vector<int> fields_;
    std::vector<Point> display_;
    std::vector<Point> position_;
};

