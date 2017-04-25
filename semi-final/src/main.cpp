#include "EagerTaxicab.h"
#include "UpwindSailer.h"
#include "SuperFill.h"
#include "Client.h"
#include <iostream>
#include <vector>
#include <cstdlib>

#include <boost/program_options.hpp>


int main(int argc, char** argv) {
	namespace po = boost::program_options;
	po::options_description desc{"Allowed Options"};
	desc.add_options()
		("help,h", "this help message")
		("host,H", po::value<std::string>(), "hostname to connect to, defaults to localhost")
		("teamname,t", po::value<std::string>(), "teamname to use during login")
		("password,p", po::value<std::string>(), "password to use for authentication")
		("level,l", po::value<int>(), "request level (defaults to random)")
		("output,o", po::value<std::string>(), "file to save server messages");

	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);
	} catch(std::exception& e) {
		std::cerr << "error: " << e.what() << std::endl;
		return 1;
	}

	/* config area */
	std::string host_name = "localhost";
	const unsigned short port = 42500;
	std::string team_name = "the_hypnotoad";
	std::string password = "******";
	std::string filename;
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

	if (vm.count("output")) {
		filename = vm["output"].as<std::string>();
	} else {
		std::cerr << "error: no output file was specified" << std::endl;
		return 1;
	}

	try {
		platform_dep::enable_socket _;
#if 0
		UpwindSailer solver;
#else
#if 0
		EagerTaxicab solver;
#else
		SuperSolver solver;
#endif
#endif
		Client(host_name, port, team_name, password, filename, level).Run(solver);

	} catch(std::exception& e) {
		std::cerr << "Exception throwed. what(): " << e.what() << std::endl;
	}
}
