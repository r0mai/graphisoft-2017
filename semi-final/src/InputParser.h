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
	std::vector<int> scores;

	// only if its our turn
	int target = -1;
	Field extra = Field(0);
};


class InputParser {
public:
	InputParser();
	std::vector<std::string> FromStream(std::istream& stream);

	FieldInfo ParseInit(const std::vector<std::string>& lines);
	TurnInfo ParseTurn(const std::vector<std::string>& lines);

private:
	FieldInfo field_info_;
	TurnInfo prev_turn_;
	std::vector<Field> extras_;
	std::vector<int> scores_;
};
