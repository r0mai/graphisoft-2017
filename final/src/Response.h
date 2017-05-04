#pragma once
#include "Point.h"
#include "Field.h"

struct Response {
	struct Push {
		Point edge;
		Field field;
	} push;

	Point move;
};

inline
std::ostream& operator<<(std::ostream& os, const Response& r) {
	os << "Push = " << r.push.edge << " (" << r.push.field << "), Move = " << r.move;
	return os;
}
