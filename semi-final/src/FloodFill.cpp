#include "FloodFill.h"

#include <stack>
#include <cassert>

#include "Util.h"

Matrix<int> FloodFill(const Matrix<Field>& fields, const Point& origin) {
	std::stack<Point> stack;
	stack.push(origin);
	auto width = fields.Width();
	auto height = fields.Height();

	Matrix<int> reachable{width, height, false};

	while (!stack.empty()) {
		Point p = stack.top();
		stack.pop();
		if (reachable.At(p)) {
			continue;
		}
		reachable.At(p) = true;

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
	return reachable;
}
