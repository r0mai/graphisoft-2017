#pragma once
#include <tuple>
#include <cstdlib>
#include <ostream>

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

inline bool operator<(const Point& lhs, const Point& rhs) {
	return std::tie(lhs.x, lhs.y) < std::tie(rhs.x, rhs.y);
}

inline bool IsValid(const Point& p) {
	return p.x != -1 || p.y != -1;
}

inline int TaxicabDistance(const Point& a, const Point& b, const Point& size) {
	auto dx = std::abs(a.x - b.x);
	auto dy = std::abs(a.y - b.y);
	return std::min(dx, size.x - dx) + std::min(dy, size.y - dy);
}

inline std::ostream& operator<<(std::ostream& os, const Point& p) {
	if (!IsValid(p)) {
		os << "(X, X)";
	} else {
		os << '(' << p.x << ", " << p.y << ')';
	}
	return os;
}
