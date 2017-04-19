#pragma once

#include "Field.h"
#include "Matrix.h"
#include "Util.h"
#include "Grid.h"

enum class Direction {
	None, Up, Down, Left, Right
};

// will contain 1 where origin is reachable (0 otherwise)
Matrix<int> FloodFill(
	const Matrix<Field>& fields,
	const Point& origin);

void FloodFillTo(
	Matrix<int>& fill_matrix,
	Matrix<Direction>& direction_matrix,
	const Matrix<Field>& fields,
	const Point& origin,
	int fill_value = 1);

void FloodFillPoints(
	Matrix<int>& fill_matrix,
	Matrix<Direction>& direction_matrix,
	const Matrix<Field>& fields,
	std::vector<Point> origins,
	int fill_value = 1);

void FloodFillExtend(
	Matrix<int>& fill_matrix,
	Matrix<Direction>& direction_matrix,
	const Matrix<Field>& fields,
	int fill_value = 1);

// coordinates with the same integer value are reachable from each other
Matrix<int> FullFloodFill(const Matrix<Field>& fields, int start_index = 1);

struct MoveChain {
	bool colored = false;
	std::vector<PushVariation> pushes;
};

Matrix<MoveChain> SuperFloodFill(
	Grid grid,
	const Point& origin,
	Field extra);
