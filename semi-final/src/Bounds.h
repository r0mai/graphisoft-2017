#pragma once

#include "Point.h"
#include "Matrix.h"

struct Bounds {
	Bounds() = default;
	Bounds(const Point& mins, const Point& maxs)
		: mins(mins), maxs(maxs) {}

	Point mins;
	Point maxs; // one larger
};

Bounds GetBounds(const Matrix<int>& m);

Bounds Grow(Bounds bounds, const Point& size);
