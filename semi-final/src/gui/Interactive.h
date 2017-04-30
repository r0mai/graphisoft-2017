#pragma once
#include "Solver.h"


class InteractiveSolver : public Solver {
public:
	void Init(int player) override;
	void Shutdown() override;
	void Update(const Grid& grid, int player) override;
	void Turn(const Grid& grid, int player, int target, Field field, Callback fn) override;
	void Idle() override;

private:
	void CheckDisplay();
	void ResetGrid(const Grid& grid, int player);
	void UpdateTitle(int player);

	int score_ = 0;
	bool has_grid_ = false;
	int displays_ = 0;
	std::vector<int> scores_;
	App app_;
	Callback callback_;
};
