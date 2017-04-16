#pragma once
#include <vector>
#include <cstdlib>
#include <cassert>
#include <iostream>

#include "Point.h"
#include "Matrix.h"

class Grid {
public:
    int Width() const;
    int Height() const;
    Point Size() const;

    void Init(int width, int height, int displays, int players);
    void Randomize();
    void UpdateFields(std::vector<int> fields);
    void UpdateDisplay(int index, const Point& pos);
    void UpdatePosition(int player, const Point& pos);
    int At(int x, int y) const;
    int Push(const Point& pos, int t);

private:
    int players_ = -1;
    std::vector<Point> display_;
    std::vector<Point> position_;
    Matrix<int> fields_;
};

