#pragma once

struct Point {
	Point() = default;
	Point(int x, int y) : x(x), y(y) {}
	int x = -1;
	int y = -1;
};
