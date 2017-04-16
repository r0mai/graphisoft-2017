#include "solver.h"

#include <iostream>
#include <sstream>
#include <stack>

// assuming north is negative X
bool NoPositiveXBorder(int type) {
	return type & 0b0001;
}

bool NoNegativeXBorder(int type) {
	return type & 0b0100;
}

bool NoPositiveYBorder(int type) {
	return type & 0b1000;
}

bool NoNegativeYBorder(int type) {
	return type & 0b0010;
}

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
			std::vector<int> fields(grid_.Width() * grid_.Height());
			for (int& f : fields) {
				ss >> f;
			}
			grid_.UpdateFields(std::move(fields));
		} else if (command == "DISPLAY") {
			int index;
			Point p;
			ss >> index >> p.x >> p.y;
			grid_.UpdateDisplay(index, p);
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

	// our turn
	std::vector<std::string> result;
	// TODO
	return result;
}

void solver::end(const std::string& message) {
	std::cerr << "We got the end message: " << message << std::endl;
}

Matrix<int> FloodFill(const Matrix<int>& field, const Point& origin) {
	std::stack<Point> stack;
	stack.push(origin);

	Matrix<int> reachable{field.Width(), field.Height(), false};

	while (!stack.empty()) {
		Point p = stack.top();
		stack.pop();
		if (reachable.At(p)) {
			continue;
		}
		reachable.At(p) = true;

		if (p.x+1 < field.Width() &&
			NoPositiveXBorder(field.At(p)) &&
			NoNegativeXBorder(field.At(p.x+1, p.y)))
		{
			stack.push({p.x+1, p.y});
		}
		if (p.x-1 >= 0 &&
			NoNegativeXBorder(field.At(p)) &&
			NoPositiveXBorder(field.At(p.x-1, p.y)))
		{
			stack.push({p.x-1, p.y});
		}
		if (p.y+1 < field.Height() &&
			NoPositiveYBorder(field.At(p)) &&
			NoNegativeYBorder(field.At(p.x, p.y+1)))
		{
			stack.push({p.x, p.y+1});
		}
		if (p.y-1 >= 0 &&
			NoNegativeYBorder(field.At(p)) &&
			NoPositiveYBorder(field.At(p.x, p.y-1)))
		{
			stack.push({p.x, p.y-1});
		}
	}
	return reachable;
}
