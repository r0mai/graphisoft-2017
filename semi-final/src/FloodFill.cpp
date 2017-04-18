#include "FloodFill.h"

#include <stack>
#include <cassert>

#include "Util.h"

Matrix<int> FloodFill(
	const Matrix<Field>& fields,
	const Point& origin)
{
	auto width = fields.Width();
	auto height = fields.Height();

	Matrix<int> reachable{width, height, 0};

	FloodFillTo(reachable, fields, origin, 1);

	return reachable;
}

void FloodFillTo(
	Matrix<int>& fill_matrix,
	const Matrix<Field>& fields,
	const Point& origin,
	int fill_value)
{
	std::stack<Point> stack;
	stack.push(origin);
	auto width = fields.Width();
	auto height = fields.Height();

	while (!stack.empty()) {
		Point p = stack.top();
		stack.pop();
		if (fill_matrix.At(p) == fill_value) {
			continue;
		}
		fill_matrix.At(p) = fill_value;

		if (p.x + 1 < width &&
			IsEastOpen(fields.At(p)) &&
			IsWestOpen(fields.At(p.x + 1, p.y)))
		{
			stack.push({p.x + 1, p.y});
		}
		if (p.x - 1 >= 0 &&
			IsWestOpen(fields.At(p)) &&
			IsEastOpen(fields.At(p.x - 1, p.y)))
		{
			stack.push({p.x - 1, p.y});
		}
		if (p.y + 1 < height &&
			IsSouthOpen(fields.At(p)) &&
			IsNorthOpen(fields.At(p.x, p.y + 1)))
		{
			stack.push({p.x, p.y + 1});
		}
		if (p.y - 1 >= 0 &&
			IsNorthOpen(fields.At(p)) &&
			IsSouthOpen(fields.At(p.x, p.y - 1)))
		{
			stack.push({p.x, p.y - 1});
		}
	}
}

Matrix<int> FullFloodFill(const Matrix<Field>& fields, int start_index) {
	auto width = fields.Width();
	auto height = fields.Height();

	Matrix<int> fill_map{width, height, 0};

	int color = start_index;
	for (int x = 0; x < fields.Width(); ++x) {
		for (int y = 0; y < fields.Height(); ++y) {
			if (fill_map.At(x, y) == 0) {
				FloodFillTo(fill_map, fields, {x, y}, color++);
			}
		}
	}
	return fill_map;
}
