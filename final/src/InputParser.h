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
	std::vector<Point> blocked_fields;
	std::vector<std::string> player_names;
	std::vector<int> target_order;
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


struct AfterInfo {
	int map_score = -1;
	int	test_score_sum = -1;
	int final_score_sum = -1;

	int game_id = -1;

	int next_map_index = -1;
	int time_until_next_map = -2; // -1 means no next game
	bool next_is_test = true;
};


class InputParser {
public:
	static const int kMaxPlayers = 10;

	InputParser();
	std::vector<std::string> FromStream(std::istream& stream);

	FieldInfo ParseInit(const std::vector<std::string>& lines);
	TurnInfo ParseTurn(const std::vector<std::string>& lines);
	AfterInfo ParseAfter(const std::vector<std::string>& lines);

private:
	FieldInfo field_info_;
	TurnInfo prev_turn_;
	std::vector<Field> extras_;
	std::vector<int> scores_;
};
