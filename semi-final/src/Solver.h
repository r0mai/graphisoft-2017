#pragma once
#include "Grid.h"
#include "Point.h"
#include "Field.h"
#include <functional>


struct Response {
	struct {
		Point edge;
		Field field;
	} push;

	Point move;
};


class Solver {
public:
	using Callback = std::function<void(const Response&)>;

	virtual void Init(int player) = 0;
	virtual void Shutdown() = 0;
	virtual void Update(const Grid& grid, int player) = 0;
	virtual void Turn(const Grid& grid, int player, int target, Field field, Callback fn) = 0;
	virtual void Idle() = 0;
};
