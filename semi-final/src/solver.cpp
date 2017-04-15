#include "solver.h"

#include <iostream>
#include <sstream>

void solver::init(const std::vector<std::string>& field_infos) {
	std::cerr << "We got these field informations:" << std::endl;

	for (auto& field : field_infos) {
		std::cerr << field << std::endl;
	}

	for (auto& field : field_infos) {
		std::stringstream ss(field);
		std::string command;
		ss >> command;
		if (command == "MESSAGE") {
			// nothing to do with this
		} else if (command == "LEVEL") {
			ss >> level_;
		} else if (command == "SIZE") {
			ss >> width_ >> height_;
		} else if (command == "DISPLAYS") {
			ss >> display_count_;
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

	for(std::size_t i = 0; i < tick_infos.size(); ++i) {
		std::cerr << tick_infos[i] << std::endl;
	}

	std::vector<std::string> result;
	// TODO ...
	// send an empty vector if not your turns
	return result;
}

void solver::end(const std::string& message) {
	std::cerr << "We got the end message: " << message << std::endl;
}

