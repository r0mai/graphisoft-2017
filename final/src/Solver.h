#pragma once
#include "Grid.h"
#include "Point.h"
#include "Field.h"
#include "Response.h"
#include <functional>



class Solver {
public:
	using Callback = std::function<void(const Response&)>;

	virtual ~Solver() {}
	virtual void Init(int player) = 0;
	virtual void Shutdown() = 0;
	virtual void Update(const Grid& grid, int player) = 0;
	virtual void Turn(const Grid& grid, int player, int target, Field field,
			int nextTarget, Callback fn) = 0;
	virtual void Idle() = 0;

	Response SyncTurn(const Grid& grid, int player, int target, Field field,
			int nextTarget);
};
