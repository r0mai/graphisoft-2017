#pragma once
#include "Solver.h"

class UpwindSailer : public Solver {
public:
	void Init(int player) override;
	void Shutdown() override;
	void Update(const Grid& grid, int player) override;
	void Turn(const Grid& grid, int player, int target, Field field, Callback fn) override;
	void Idle() override;

private:
	Grid grid;
	Field extra;
	int player;
	int target_display;
};

Response UpwindSailerStep(const Grid& newGrid, int player, int target, Field field);
