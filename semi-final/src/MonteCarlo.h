#pragma once
#include "Solver.h"

class MonteCarlo : public Solver {
public:
	void Init(int player) override {}
	void Shutdown() override {}
	void Update(const Grid& grid, int player) override {}
	void Turn(const Grid& grid, int player, int target, Field field, Callback fn) override;
	void Idle() override {}

private:

};
