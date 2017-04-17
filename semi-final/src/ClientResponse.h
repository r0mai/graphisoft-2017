#pragma once

#include "Field.h"
#include "Point.h"

struct ClientResponse {
	// push
	struct {
		Point direction;
		Field field;
	} push;

	// goto
	struct {
		Point target;
	} move;
};
