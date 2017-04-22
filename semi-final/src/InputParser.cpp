#include "InputParser.h"
#include "Point.h"
#include <sstream>


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
		field_info_.displays, 4);

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
		}
	}

	assert(info.tick >= 0);
	assert(info.player >= 0);
	info.opponent = info.player != field_info_.player;
	if (!info.opponent) {
		assert(info.target >= 0);
		assert(info.extra > 0);
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
