#include "solver.h"

#include <iostream>
#include <sstream>

void solver::init(const std::vector<std::string>& field_infos) {
	std::cerr << "We got these field informations:" << std::endl;

	for (auto& info : field_infos) {
		std::cerr << info << std::endl;
	}

	int width = -1;
	int height = -1;
	int players = 4;
	int displays = -1;

	for (auto& info : field_infos) {
		std::stringstream ss(info);
		std::string command;
		ss >> command;
		if (command == "MESSAGE") {
			// nothing to do with this
		} else if (command == "LEVEL") {
			ss >> level_;
		} else if (command == "SIZE") {
			ss >> width >> height;
		} else if (command == "DISPLAYS") {
			ss >> displays;
		} else if (command == "PLAYER") {
			ss >> player_index_;
		} else if (command == "MAXTICK") {
			ss >> max_tick_;
		} else {
			std::cerr << "WARNING: unhandled command: " << command << std::endl;
		}
	}

	assert(width > 0);
	assert(height > 0);
	assert(displays >= 0);

	grid_.Init(width, height, displays, players);
}

std::vector<std::string> solver::process(const std::vector<std::string>& tick_infos) {
	std::cerr << "We got these tick informations:" << std::endl;

	for (auto& info : tick_infos) {
		std::cerr << info << std::endl;
	}

	for (auto& info : tick_infos) {
		std::stringstream ss(info);
		std::string command;
		ss >> command;
		if (command == "MESSAGE") {
			// nothing to do with this
		} else if (command == "TICK") {
			ss >> current_tick_;
		} else if (command == "FIELDS") {
			std::vector<Field> fields(grid_.Width() * grid_.Height());
			for (Field& f : fields) {
				ss >> f;
			}
			grid_.UpdateFields(std::move(fields));
		} else if (command == "DISPLAY") {
			int index;
			Point p;
			ss >> index >> p.x >> p.y;
			grid_.UpdateDisplay(index, p);
		} else if (command == "POSITION") {
			int index;
			Point p;
			ss >> index >> p.x >> p.y;
			grid_.UpdatePosition(index, p);
		} else if (command == "PLAYER") {
			ss >> current_player_;
		} else if (command == "TARGET") {
			ss >> target_display_;
		} else if (command == "EXTRAFIELD") {
			ss >> extra_field_;
		}
	}

	if (player_index_ != current_player_) {
		// not our turn
		return {};
	}

	return ClientResponseToStrings(MVPAI());
}

ClientResponse solver::MVPAI() {
	ClientResponse response;
	response.push.direction = {-1, 0};
	response.push.field = extra_field_;
	response.move.target = grid_.Positions()[player_index_];
	return response;
}

std::vector<std::string> solver::ClientResponseToStrings(
	const ClientResponse& response) const
{
	std::vector<std::string> strings;

	int c, p, k, t;
	if (response.push.direction.x == -1) {
		c = 0;
		p = 1;
	} else if (response.push.direction.x == grid_.Width()) {
		c = 0;
		p = 0;
	} else if (response.push.direction.y == -1) {
		c = 1;
		p = 1;
	} else if (response.push.direction.y == grid_.Height()) {
		c = 1;
		p = 0;
	} else {
		assert(false);
	}
	if (c == 0) {
		k = response.push.direction.y;
	} else {
		k = response.push.direction.x;
	}
	t = response.push.field;

	{
		std::stringstream ss;
		ss << "PUSH " << c << " " << p << " " << k << " " << t;
		strings.push_back(ss.str());
	}
	{
		std::stringstream ss;
		ss << "GOTO " << response.move.target.x << " " << response.move.target.y;
		strings.push_back(ss.str());
	}
	return strings;
}

void solver::end(const std::string& message) {
	std::cerr << "We got the end message: " << message << std::endl;
}
