#pragma once
#include <tuple>
#include <cstdlib>
#include <ostream>
#include <algorithm>

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

inline std::ostream& operator<<(std::ostream& os, const Point& p) {
	return os << '(' << p.x << ", " << p.y << ')';
}

inline bool IsValid(const Point& p) {
	return p.x != -1 || p.y != -1;
}

inline int TaxicabDistance(const Point& a, const Point& b) {
	return std::labs(a.x - b.x) + std::labs(a.y - b.y);
}

// [] style clamp
inline Point Clamp(const Point& p, const Point& min, const Point& max) {
	return {
		std::max(min.x, std::min(p.x, max.x)),
		std::max(min.y, std::min(p.y, max.y))
	};
}

// [) style clamp
inline Point Clamp2(const Point& p, const Point& min, const Point& max) {
	return {
		std::max(min.x, std::min(p.x, max.x-1)),
		std::max(min.y, std::min(p.y, max.y-1))
	};
}
