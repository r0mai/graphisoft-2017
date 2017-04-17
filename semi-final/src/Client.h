#pragma once
#include "platform_dep.h"
#include "solver.h"
#include <string>
#include <sstream>
#include <vector>


class Client {
public:
	Client(const char host_name[], unsigned short port,
		const char team_name[], const char password[], int task_id);

	void SendMessages(const std::vector<std::string>& messages);
	std::vector<std::string> ReceiveMessage();
	void Run();

private:
	platform_dep::tcp_socket socket_handler;
	std::string received_buffer;
	solver your_solver;
};
