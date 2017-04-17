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
	int DisplayCount() const;
	Point Size() const;
	const Matrix<Field>& Fields() const;
	const std::vector<Point>& Displays() const;
	const std::vector<Point>& Positions() const;

	void Init(int width, int height, int displays, int players);
	void Randomize();
	void ResetDisplays();
	void UpdateFields(std::vector<Field> fields);
	void UpdateDisplay(int index, const Point& pos);
	void UpdatePosition(int player, const Point& pos);
	Field At(int x, int y) const;
	Field Push(const Point& pos, Field t);
	Field Push(int c, int p, int k, Field t);

private:
	std::vector<Point> displays_;
	std::vector<Point> positions_;
	Matrix<Field> fields_;
};

std::ostream& operator<<(std::ostream& os, const Grid& grid);
