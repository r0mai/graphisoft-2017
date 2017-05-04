#pragma once
#include "Solver.h"

class EagerTaxicab : public Solver {
public:
	void Init(int player) override {}
	void Shutdown() override {}
	void Update(const Grid& grid, int player) override {}
	void Turn(const Grid& grid, int player, int target, Field field, Callback fn) override;
	void Idle() override {}

private:
	Response GetResponse();

	// <distance from target after move, move posiiton>
	std::tuple<int, Point> MoveClosestToTarget();

	Grid grid_;
	Field extra_;
	int player_ = -1;
	int target_ = -1;
};
