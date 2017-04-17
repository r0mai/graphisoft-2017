#include "solver.h"
#include "Client.h"
#include <iostream>
#include <vector>
#include <cstdlib>

#include <boost/program_options.hpp>


int main(int argc, char** argv) {
	namespace po = boost::program_options;
	po::options_description desc{"Allowed Options"};
	desc.add_options()
		("help", "this help message")
		("host", po::value<std::string>(), "hostname to connect to, defaults to localhost")
		("teamname", po::value<std::string>(), "teamname to use during login")
		("password", po::value<std::string>(), "password to use for authentication");
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
	/* config area */
	std::string host_name = "localhost";
	const unsigned short port = 42500;
	std::string team_name = "CSAPATNEVETEK";
	std::string password = "JELSZAVATOK";

	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return 0;
	}

	if (vm.count("host")) {
		host_name = vm["host"].as<std::string>();
	}

	if (vm.count("teamname")) {
		team_name = vm["teamname"].as<std::string>();
	}

	if (vm.count("password")) {
		password = vm["password"].as<std::string>();
	}

	// 0 means here: server choose randomly 1 to 10
	const int task_id = argc > 1 ? std::atoi(argv[1]) : 0;

    try {
    	platform_dep::enable_socket _;

	    Client(host_name.c_str(), port, team_name.c_str(),
				password.c_str(), task_id).Run();

    } catch(std::exception& e) {
        std::cerr << "Exception throwed. what(): " << e.what() << std::endl;
    }
}
