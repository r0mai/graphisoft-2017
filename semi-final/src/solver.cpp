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

	for (auto& info : field_infos) {
		std::stringstream ss(info);
		std::string command;
		ss >> command;
		if (command == "MESSAGE") {
			// nothing to do with this
		} else if (command == "LEVEL") {
			ss >> level_;
		} else if (command == "SIZE") {
			ss >> width_ >> height_;
			tick.field.resize(width_);
			for (auto& col : tick.field) {
				col.resize(height_);
			}
		} else if (command == "DISPLAYS") {
			ss >> display_count_;
			tick.displays.resize(display_count_);
		} else if (command == "PLAYER") {
			ss >> player_index_;
		} else if (command == "MAXTICK") {
			ss >> max_tick_;
		} else {
			std::cerr << "WARNING: unhandled command: " << command << std::endl;
		}
	}
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
			ss >> tick.tick;
		} else if (command == "FIELDS") {
			for (int y = 0; y < height_; ++y) {
				for (int x = 0; x < width_; ++x) {
					ss >> tick.field[x][y];
				}
			}
		} else if (command == "DISPLAY") {
			int index;
			ss >> index;
			ss >> tick.displays[index].x >> tick.displays[index].y;
		} else if (command == "PLAYER") {
			ss >> tick.current_player;
		} else if (command == "TARGET") {
			ss >> tick.target_display;
		} else if (command == "EXTRAFIELD") {
			ss >> tick.our_field_type;
		}
	}

	if (player_index_ != tick.current_player) {
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

Matrix<bool> FloodFill(const Matrix<int>& field, const Point& origin) {
	int w = field.size();
	int h = field[0].size();

	std::stack<Point> stack;
	stack.push(origin);

	Matrix<bool> reachable = CloneMatrixDimensions(field, false);

	while (!stack.empty()) {
		Point p = stack.top();
		stack.pop();
		if (reachable[p.x][p.y]) {
			continue;
		}
		reachable[p.x][p.y] = true;

		if (p.x+1 < w &&
			NoPositiveXBorder(field[p.x][p.y]) &&
			NoNegativeXBorder(field[p.x+1][p.y]))
		{
			stack.push({p.x+1, p.y});
		}
		if (p.x-1 >= 0 &&
			NoNegativeXBorder(field[p.x][p.y]) &&
			NoPositiveXBorder(field[p.x-1][p.y]))
		{
			stack.push({p.x-1, p.y});
		}
		if (p.y+1 < h &&
			NoPositiveYBorder(field[p.x][p.y]) &&
			NoNegativeYBorder(field[p.x][p.y+1]))
		{
			stack.push({p.x, p.y+1});
		}
		if (p.y-1 >= 0 &&
			NoNegativeYBorder(field[p.x][p.y]) &&
			NoPositiveYBorder(field[p.x][p.y-1]))
		{
			stack.push({p.x, p.y-1});
		}
	}
	return reachable;
}
