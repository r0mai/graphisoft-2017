#pragma once
#include "platform_dep.h"
#include "Solver.h"
#include <string>
#include <sstream>
#include <vector>


class Client {
public:
	Client(
		const std::string& host_name, int port,
		const std::string& team_name, const std::string& password, int task_id);

	void Run(Solver& solver);

private:
	void Init(const std::vector<std::string>& field_infos, Solver& solver);
	bool Process(const std::vector<std::string>& tick_infos, Solver& solver);

	void SendMessages(const std::vector<std::string>& messages);
	std::vector<std::string> FromResponse(const Response& response) const;
	std::vector<std::string> ReceiveMessage();

	platform_dep::tcp_socket socket_handler_;
	std::string received_buffer_;

	Grid grid_;
	int player_index_ = -1;
	bool wait_ = false;
	bool opponent_ = false;
	Response response_;
};
