#include "EagerTaxicab.h"
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
		("password", po::value<std::string>(), "password to use for authentication")
		("level", po::value<int>(), "request level (defaults to random)") ;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
	/* config area */
	std::string host_name = "localhost";
	const unsigned short port = 42500;
	std::string team_name = "taxicab";
	std::string password = "******";
	int level = 0;

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

	if (vm.count("level")) {
		level = vm["level"].as<int>();
	}

	try {
		platform_dep::enable_socket _;
		EagerTaxicab solver;

		Client(host_name.c_str(), port, team_name.c_str(),
				password.c_str(), level).Run(solver);

	} catch(std::exception& e) {
		std::cerr << "Exception throwed. what(): " << e.what() << std::endl;
	}
}
