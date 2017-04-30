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
