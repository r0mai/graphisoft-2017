#pragma once
#include "platform_dep.h"
#include "Solver.h"
#include "InputParser.h"
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <boost/optional.hpp>


class Client {
public:
	Client(
		const std::string& host_name, int port,
		const std::string& team_name, const std::string& password,
		const std::string& filename = {}, int level = 0, bool verbose=false);

	void Run(Solver& solver);

private:
	void Init(const std::vector<std::string>& field_infos, Solver& solver);
	bool Process(const std::vector<std::string>& tick_infos, Solver& solver);

	void SendMessages(const std::vector<std::string>& messages);
	std::vector<std::string> FromResponse(const Response& response) const;
	boost::optional<std::vector<std::string>> CheckForMessage();
	std::vector<std::string> ReceiveMessage();

	void SaveInput(const std::vector<std::string>& lines);
	void BlockUntilMessageArrives();
	void BlockUntilMessageCanBeSent();

	platform_dep::tcp_socket socket_handler_;
	std::string received_buffer_;

	InputParser parser_;
	Grid grid_;
	int player_index_ = -1;
	bool wait_ = false;
	bool opponent_ = false;
	bool verbose_ = false;
	Response response_;
	std::ofstream output_;
};
