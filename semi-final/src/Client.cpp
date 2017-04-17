#include "Client.h"
#include <vector>
#include <iostream>


Client::Client(
	const std::string& host_name, int port,
	const std::string& team_name, const std::string& password, int task_id) {

	if(!socket_handler_.valid()) {
		throw std::runtime_error("Error: Cannot open a socket!");
	}

	hostent* host = gethostbyname(host_name.c_str());

	if(!host) {
		throw std::runtime_error("Error: Cannot find host: " + host_name);
	}

	sockaddr_in socket_address;
	socket_address.sin_family = AF_INET;
	socket_address.sin_port = htons(port);
	socket_address.sin_addr.s_addr = inet_addr(inet_ntoa(*(in_addr *)host->h_addr_list[0]));

	if(connect(socket_handler_.get_handler(), (struct sockaddr*)&socket_address,
		sizeof(socket_address)) != 0) {
		throw std::runtime_error(
			"Error: Cannot connect to the server: " +
			host_name + ":" + std::to_string(port) +
			" error code: " + std::to_string(socketerrno));
	}

	std::vector<std::string> login_messages;

	login_messages.push_back(std::string("LOGIN ") + team_name + " " + password);

	if(1 <= task_id && task_id <= 10) {
		login_messages.push_back("LEVEL " + std::to_string(task_id));
	}

	SendMessages(login_messages);
}

void Client::SendMessages(const std::vector<std::string>& messages) {
	std::string message;

	for(std::size_t i = 0; i < messages.size(); ++i) {
		message += messages[i] + '\n';
	}
	message += ".\n";

	int sent_bytes = send(socket_handler_.get_handler(), message.c_str(), message.size(), 0);

	if(sent_bytes != (int)message.size()) {
		std::cerr << "Warning: Cannot sent message properly: " << message << std::endl;
		std::cerr << sent_bytes << " byte sent from " <<
			message.size() << ". Closing connection." << std::endl;
		socket_handler_.invalidate();
	}
}

std::vector<std::string> Client::ReceiveMessage() {
	std::string buffer(512, '\0');

	int received_bytes = recv(socket_handler_.get_handler(), &buffer[0], 512, 0);

	switch(received_bytes) {
	case -1:
		std::cerr << "Error: recv failed!" << std::endl;
	case 0:
		std::cerr << "Connection closed." << std::endl;
		socket_handler_.invalidate();
		return std::vector<std::string>();
	}

	std::vector<std::string> result;

	std::stringstream consumer(received_buffer_ + buffer.c_str());
	while(std::getline(consumer, buffer)) {
		if(buffer == ".") {
			received_buffer_ = consumer.str().substr(consumer.tellg());
			return result;
		} else if(!buffer.empty()) {
			result.push_back(buffer);
		}
	}

	received_buffer_ = consumer.str();
	return ReceiveMessage();
}

void Client::Init(const std::vector<std::string>& field_infos, Solver& solver) {
	std::cerr << "We got these field informations:" << std::endl;

	for (auto& info : field_infos) {
		std::cerr << info << std::endl;
	}

	int width = -1;
	int height = -1;
	int players = 4;
	int displays = -1;

	int level = -1;
	int max_tick = -1;

	for (auto& info : field_infos) {
		std::stringstream ss(info);
		std::string command;
		ss >> command;
		if (command == "MESSAGE") {
			// nothing to do with this
		} else if (command == "LEVEL") {
			ss >> level;
		} else if (command == "SIZE") {
			ss >> width >> height;
		} else if (command == "DISPLAYS") {
			ss >> displays;
		} else if (command == "PLAYER") {
			ss >> player_index_;
		} else if (command == "MAXTICK") {
			ss >> max_tick;
		} else {
			std::cerr << "WARNING: unhandled command: " << command << std::endl;
		}
	}

	assert(width > 0);
	assert(height > 0);
	assert(displays >= 0);
	assert(max_tick > 0);
	assert(player_index_ >= 0);

	grid_.Init(width, height, displays, players);
	solver.Init(player_index_);
}

bool Client::Process(const std::vector<std::string>& tick_infos, Solver& solver) {
	if (tick_infos.size() == 1 && tick_infos[0].find("END") == 0) {
		std::cerr << "We got the end message: " << tick_infos[0] << std::endl;
		solver.Shutdown();
		return false;
	}
	std::cerr << "We got these tick informations:" << std::endl;

	for (auto& info : tick_infos) {
		std::cerr << info << std::endl;
	}

	int current_tick = -1;
	int current_player = -1;
	int target_display = -1;
	int extra_field = -1;

	grid_.ResetDisplays();
	for (auto& info : tick_infos) {
		std::stringstream ss(info);
		std::string command;
		ss >> command;
		if (command == "MESSAGE") {
			// nothing to do with this
		} else if (command == "TICK") {
			ss >> current_tick;
		} else if (command == "FIELDS") {
			std::vector<Field> fields(grid_.Width() * grid_.Height());
			for (Field& f : fields) {
				ss >> f;
			}
			grid_.UpdateFields(std::move(fields));
		} else if (command == "DISPLAY") {
			int index;
			Point p;
			ss >> index >> p.x >> p.y;
			grid_.UpdateDisplay(index, p);
		} else if (command == "POSITION") {
			int index;
			Point p;
			ss >> index >> p.x >> p.y;
			grid_.UpdatePosition(index, p);
		} else if (command == "PLAYER") {
			ss >> current_player;
		} else if (command == "TARGET") {
			ss >> target_display;
		} else if (command == "EXTRAFIELD") {
			ss >> extra_field;
		}
	}

	assert(current_tick >= 0);
	assert(current_player >= 0);
	opponent_ = (player_index_ != current_player);

	if (player_index_ != current_player) {
		solver.Update(grid_, current_player);
	} else {
		assert(target_display >= 0);
		assert(extra_field > 0);

		wait_ = true;
		solver.Turn(grid_, current_player, target_display, Field(extra_field),
			[&](const Response& response) {
				wait_ = false;
				response_ = response;
			});
	}
	return true;
}

std::vector<std::string> Client::FromResponse(const Response& response) const {
	std::vector<std::string> strings;

	int c, p, k, t;
	if (response.push.edge.x == -1) {
		c = 0;
		p = 1;
	} else if (response.push.edge.x == grid_.Width()) {
		c = 0;
		p = 0;
	} else if (response.push.edge.y == -1) {
		c = 1;
		p = 1;
	} else if (response.push.edge.y == grid_.Height()) {
		c = 1;
		p = 0;
	} else {
		assert(false);
	}

	k = (c == 0
		? response.push.edge.y
		: response.push.edge.x);
	t = response.push.field;

	if (true) {
		std::stringstream ss;
		ss << "PUSH " << c << " " << p << " " << k << " " << t;
		strings.push_back(ss.str());
	}

	if (IsValid(response.move)) {
		std::stringstream ss;
		ss << "GOTO " << response.move.x << " " << response.move.y;
		strings.push_back(ss.str());
	}
	return strings;
}

void Client::Run(Solver& solver) {
	std::vector<std::string> msg = ReceiveMessage();

	if (socket_handler_.valid()) {
		Init(msg, solver);
	}

	while (socket_handler_.valid()) {
		msg = ReceiveMessage();
		if (socket_handler_.valid()) {
			if (!Process(msg, solver)) {
				break;
			}
			if (opponent_) {
				continue;
			}

			while (wait_) {
				solver.Idle();
			}

			if (socket_handler_.valid()) {
				msg = FromResponse(response_);
				SendMessages(msg);
			}
		}
	}
	std::cerr << "Game over" << std::endl;
}
