#include "solver.h"

#include <iostream>

void solver::init(const std::vector<std::string>& field_infos) {
	std::cerr << "We got these field informations:" << std::endl;

	for(std::size_t i = 0; i < field_infos.size(); ++i) {
		std::cerr << field_infos[i] << std::endl;
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

