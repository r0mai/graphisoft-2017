#include "Game.h"
#include "GameWindow.h"

#include <iostream>
#include <boost/program_options.hpp>


int main(int argc, char* argv[]) {
	namespace po = boost::program_options;
	po::options_description desc{"Allowed Options"};
	desc.add_options()
		("help,h", "this help message")
		("hotseat,s", po::value<int>()->value_name("N"), "hotseat mode with N players")
		("host,H", po::value<std::string>(), "hostname to connect, defaults to localhost")
		("team,t", po::value<std::string>(), "teamname to use during login")
		("password,p", po::value<std::string>(), "password to use for authentication")
		("replay,r", po::value<std::string>(), "replay from file");
	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);
	} catch(std::exception& e) {
		std::cerr << "error: " << e.what() << std::endl;
		return 1;
	}

	bool is_hotseat = vm.count("hotseat");
	bool is_replay = false;

	int port = 42500;
	std::string host_name = "localhost";
	std::string team_name = "the_hypnotoad";
	std::string password = "******";
	std::string filename;

	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return 0;
	}

	if (vm.count("host")) {
		host_name = vm["host"].as<std::string>();
	}

	if (vm.count("team")) {
		team_name = vm["team"].as<std::string>();
	}

	if (vm.count("password")) {
		password = vm["password"].as<std::string>();
	}

	if (vm.count("replay")) {
		filename = vm["replay"].as<std::string>();
		is_replay = true;
	}

	if (is_hotseat) {
		// int players = vm["hotseat"].as<int>();
		// HotSeat(players);
	} else if (is_replay) {
		Game game;
		GameWindow game_window(game);
		game.InitReplay(filename);

		while (game_window.IsOpen()) {
			game_window.ProcessEvents();
			game.Update();
			game_window.Draw();
		}
	} else {
	// 	try {
	// 		platform_dep::enable_socket _;
	// 		InteractiveSolver solver;
	// 		Client(host_name, port, team_name, password).Run(solver);
	// 	} catch(std::exception& e) {
	// 		std::cerr << "Exception thrown. what(): " << e.what() << std::endl;
	// 	}
	}

	return 0;
}
