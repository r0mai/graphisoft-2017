#pragma once
#include <vector>
#include <cstdlib>
#include <cassert>
#include <iostream>

#include "Point.h"
#include "Matrix.h"
#include "Field.h"

class Grid {
public:
	int Width() const;
	int Height() const;
	Point Size() const;

	void Init(int width, int height, int displays, int players);
	void Randomize();
	void UpdateFields(std::vector<Field> fields);
	void UpdateDisplay(int index, const Point& pos);
	void UpdatePosition(int player, const Point& pos);
	Field At(int x, int y) const;
	Field Push(const Point& pos, Field t);

private:
	std::vector<Point> display_;
	std::vector<Point> position_;
	Matrix<Field> fields_;
};

