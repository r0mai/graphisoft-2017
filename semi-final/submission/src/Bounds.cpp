#include "Bounds.h"

#include <algorithm>

Bounds GetBounds(const Matrix<int>& m) {
	Bounds bounds;
	bounds.mins = Point(m.Width(), m.Height());
	bounds.maxs = Point(-1, -1);
	for (int y = 0; y < m.Height(); ++y) {
		for (int x = 0; x < m.Width(); ++x) {
			if (m.At(x, y)) {
				if (x < bounds.mins.x) { bounds.mins.x = x; }
				if (x > bounds.maxs.x) { bounds.maxs.x = x; }
				if (y < bounds.mins.y) { bounds.mins.y = y; }
				if (y > bounds.maxs.y) { bounds.maxs.y = y; }
			}
		}
	}
	bounds.maxs.x++;
	bounds.maxs.y++;
	return bounds;
}

Bounds Grow(Bounds bounds, const Point& size) {
	bounds.mins.x = std::max(0, bounds.mins.x - 1);
	bounds.mins.y = std::max(0, bounds.mins.y - 1);

	bounds.maxs.x = std::min(size.x, bounds.maxs.x + 1);
	bounds.maxs.y = std::min(size.y, bounds.maxs.y + 1);

	return bounds;
}
