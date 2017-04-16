#pragma once
#include <tuple>


struct Point {
	Point() = default;
	Point(int x, int y) : x(x), y(y) {}
	int x = -1;
	int y = -1;
};

inline bool operator==(const Point& lhs, const Point& rhs) {
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

inline bool operator!=(const Point& lhs, const Point& rhs) {
	return !(lhs == rhs);
}
