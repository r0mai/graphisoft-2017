#include "InputParser.h"
#include "Point.h"
#include <sstream>

InputParser::InputParser()
	: extras_(kMaxPlayers, Field(15))
	, scores_(kMaxPlayers, 0)
{}

FieldInfo InputParser::ParseInit(const std::vector<std::string>& info_lines) {
	FieldInfo info;

	for (auto& line : info_lines) {
		std::stringstream ss(line);
		std::string command;
		ss >> command;
		if (command == "MESSAGE") {
			// nothing to do with this
		} else if (command == "LEVEL") {
			ss >> info.level;
		} else if (command == "SIZE") {
			ss >> info.width >> info.height;
		} else if (command == "DISPLAYS") {
			ss >> info.displays;
		} else if (command == "PLAYER") {
			ss >> info.player;
		} else if (command == "PLAYERS") {
			std::string player;
			while (ss >> player) {
				info.player_names.push_back(player);
			}
		} else if (command == "BLOCKED") {
			Point p;
			ss >> p.x >> p.y;
			info.blocked_fields.push_back(p);
		} else if (command == "TARGETS") {
			info.target_order.resize(info.displays);
			for (int i = 0; i < info.displays; ++i) {
				ss >> info.target_order[i];
			}
		} else if (command == "MAXTICK") {
			ss >> info.max_tick;
		} else {
			std::cerr << "WARNING: unhandled command: " << command << std::endl;
		}
	}

	assert(info.width > 0);
	assert(info.height > 0);
	assert(info.displays >= 0);
	assert(info.max_tick > 0);
	assert(info.player >= 0);
	field_info_ = info;
	return info;
}

TurnInfo InputParser::ParseTurn(const std::vector<std::string>& info_lines) {
	TurnInfo info;

	if (info_lines.size() == 1 && info_lines[0].find("END") == 0) {
		info.end = true;
		return info;
	}

	info.grid.Init(
		field_info_.width, field_info_.height,
		field_info_.displays, kMaxPlayers);

	for (const auto& pos : field_info_.blocked_fields) {
		info.grid.AddBlocked(pos.x, pos.y);
	}

	for (auto& line : info_lines) {
		std::stringstream ss(line);
		std::string command;
		ss >> command;
		if (command == "MESSAGE") {
			// nothing to do with this
		} else if (command == "TICK") {
			ss >> info.tick;
		} else if (command == "FIELDS") {
			std::vector<Field> fields(field_info_.width * field_info_.height);
			for (Field& f : fields) {
				ss >> f;
			}
			info.grid.UpdateFields(std::move(fields));
		} else if (command == "DISPLAY") {
			int index;
			Point p;
			ss >> index >> p.x >> p.y;
			info.grid.UpdateDisplay(index, p);
		} else if (command == "POSITION") {
			int index;
			Point p;
			ss >> index >> p.x >> p.y;
			info.grid.UpdatePosition(index, p);
		} else if (command == "PLAYER") {
			ss >> info.player;
		} else if (command == "TARGET") {
			ss >> info.target;
		} else if (command == "EXTRAFIELD") {
			ss >> info.extra;
		} else if (command == "GAMESCORE") {
			// TODO if we need it at all
		}
	}

	if (info.extra == Field(0)) {
		info.extra = extras_[info.player];
	}

	assert(info.tick >= 0);
	assert(info.player >= 0);
	info.opponent = info.player != field_info_.player;
	if (!info.opponent) {
		assert(info.target >= 0);
		assert(info.extra > 0);
	}

	if (prev_turn_.tick >= 0) {
		auto& prev_extra = extras_[prev_turn_.player];
		prev_extra = info.grid.TileDiff(prev_turn_.grid, prev_extra);
		auto scored = info.grid.ScoreDiff(prev_turn_.grid);
		if (scored) {
			++scores_[prev_turn_.player];
		}
	}
	info.scores = scores_;

	prev_turn_ = info;
	return info;
}


AfterInfo InputParser::ParseAfter(const std::vector<std::string>& lines) {
	AfterInfo info;
	prev_turn_ = {};
	for (auto& line : lines) {
		std::cerr << "PARSEAFTER : " << line << std::endl;
		std::stringstream ss(line);
		std::string command;
		ss >> command;
		if (command == "SCORE") {
			ss >> info.map_score >> info.test_score_sum >> info.final_score_sum;
		} else if (command == "ID") {
			ss >> info.game_id;
		} else if  (command == "NEXTSTART") {
			ss >> info.next_map_index;
			ss >> info.time_until_next_map;
			int b;
			ss >> b;
			info.next_is_test = (b == 0);
		}

	}
	return info;
}


std::vector<std::string> InputParser::FromStream(std::istream& stream) {
	std::vector<std::string> lines;
	std::string line;
	while (std::getline(stream, line)) {
		if (line == ".") {
			break;
		} else {
			lines.push_back(line);
		}
	}
	return lines;
}
