#include "FloodFill.h"

#include <stack>
#include <cassert>

#include "Util.h"

Matrix<int> FloodFill(const Matrix<int>& field, const Point& origin) {
	std::stack<Point> stack;
	stack.push(origin);

	Matrix<int> reachable{field.Width(), field.Height(), false};

	while (!stack.empty()) {
		Point p = stack.top();
		stack.pop();
		if (reachable.At(p)) {
			continue;
		}
		reachable.At(p) = true;

		if (p.x+1 < field.Width() &&
			NoPositiveXBorder(field.At(p)) &&
			NoNegativeXBorder(field.At(p.x+1, p.y)))
		{
			stack.push({p.x+1, p.y});
		}
		if (p.x-1 >= 0 &&
			NoNegativeXBorder(field.At(p)) &&
			NoPositiveXBorder(field.At(p.x-1, p.y)))
		{
			stack.push({p.x-1, p.y});
		}
		if (p.y+1 < field.Height() &&
			NoPositiveYBorder(field.At(p)) &&
			NoNegativeYBorder(field.At(p.x, p.y+1)))
		{
			stack.push({p.x, p.y+1});
		}
		if (p.y-1 >= 0 &&
			NoNegativeYBorder(field.At(p)) &&
			NoPositiveYBorder(field.At(p.x, p.y-1)))
		{
			stack.push({p.x, p.y-1});
		}
	}
	return reachable;
}
