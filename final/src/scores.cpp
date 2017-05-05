#include "Matrix.h"
#include "InputParser.h"

#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <map>


struct Session {
	std::map<std::string, int> score;
	std::map<std::string, std::vector<int>> gamescore;
};


void UpdateScores(int index, const std::string& filename, Session& session) {
	try {
		InputParser parser;
		std::ifstream input(filename);
		input.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		auto info = parser.ParseInit(parser.FromStream(input));

		TurnInfo turn;
		int player_count = 0;
		while (!(turn = parser.ParseTurn(parser.FromStream(input))).score) {}

		// assume last player scored
		turn.scores[turn.player] += 1;

		int i = -1;
		for (auto x : turn.scores) {
			++i;
			const auto& name = info.player_names[i];
			auto& score = session.score[name];

			score += x;
			session.gamescore[name].push_back(score);
		}

		// std::cout << "GAME " << index << std::endl;
		// for (auto kv : scores) {
		// 	std::cout << "  " << kv.first << ": " << kv.second << std::endl;
		// }

		// std::cout << std::endl;

	} catch (std::ifstream::failure ex) {
		std::cerr << "error: cannot read file: " << filename << std::endl;
		exit(1);
	}
}


int main() {
	Session session;


	for (int i = 1; i <= 15; ++i) {
		std::stringstream ss;
		ss << std::setfill('0') << std::setw(2) << i;
		auto fname = "games/game_" + ss.str() + ".log";
		// std::cout << fname << std::endl;
		UpdateScores(i, fname, session);
	}

	for (const auto& p : session.gamescore) {
		std::cout << p.first;
		for (auto x : p.second) {
			std::cout << "\t" << x;
		}
		std::cout << std::endl;
	}
}
