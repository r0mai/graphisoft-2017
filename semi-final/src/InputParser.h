#pragma once
#include <string>
#include <vector>
#include "Field.h"
#include "Grid.h"


struct FieldInfo {
	int width = -1;
	int height = -1;
	int displays = -1;
	int level = -1;
	int max_tick = -1;
	int player = -1;
};


struct TurnInfo {
	bool end = false;
	bool opponent = false;
	int tick = -1;
	int player = -1;
	Grid grid;

	// only if its our turn
	int target = -1;
	Field extra = Field(0);
};


class InputParser {
public:
	FieldInfo ParseInit(const std::vector<std::string>& lines);
	TurnInfo ParseTurn(const std::vector<std::string>& lines);

private:
	FieldInfo field_info_;
};
