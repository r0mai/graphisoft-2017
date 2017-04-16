#pragma once

#include "Matrix.h"

// return type is matrix of bool, but it's int for vector<bool> reasons
Matrix<int> FloodFill(const Matrix<int>& field, const Point& origin);
