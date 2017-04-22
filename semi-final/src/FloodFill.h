#pragma once

#include "Field.h"
#include "Matrix.h"
#include "Grid.h"

// will contain 1 where origin is reachable (0 otherwise)
Matrix<int> FloodFill(
	const Matrix<Field>& fields,
	const Point& origin);

void FloodFillTo(
	Matrix<int>& fill_matrix,
	const Matrix<Field>& fields,
	const Point& origin,
	int fill_value = 1);

void FloodFillTo(
	Matrix<int>& fill_matrix,
	const Matrix<Field>& fields,
	std::vector<Point> origins,
	int fill_value = 1);


// coordinates with the same integer value are reachable from each other
Matrix<int> FullFloodFill(const Matrix<Field>& fields, int start_index = 1);
Matrix<int> StupidFloodFill(Grid grid, const Point& origin, Field extra);

