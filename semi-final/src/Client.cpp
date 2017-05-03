#include "Client.h"
#include <vector>
#include <iostream>

#include "fcntl.h"
#include "string.h"

#define VERBOSE 0

Client::Client(
	const std::string& host_name, int port,
	const std::string& team_name, const std::string& password,
	const std::string& filename, int level) {

	if(!socket_handler_.valid()) {
		throw std::runtime_error("Error: Cannot open a socket!");
	}

	hostent* host = gethostbyname(host_name.c_str());

	if(!host) {
		throw std::runtime_error("Error: Cannot find host: " + host_name);
	}

	output_.open(filename);
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

	int flags = ::fcntl(socket_handler_.get_handler(), F_GETFL, 0);
	flags |= O_NONBLOCK;
	::fcntl(socket_handler_.get_handler(), F_SETFL, flags);

	std::vector<std::string> login_messages;

	login_messages.push_back(std::string("LOGIN ") + team_name + " " + password);

	if(1 <= level && level <= 10) {
		login_messages.push_back("LEVEL " + std::to_string(level));
	}

	SendMessages(login_messages);
}

void Client::SendMessages(const std::vector<std::string>& messages) {
	std::string message;

	for(std::size_t i = 0; i < messages.size(); ++i) {
		message += messages[i] + '\n';
	}
	message += ".\n";

#if VERBOSE
	std::cerr << "Will try to send: " << message << std::endl;
#endif
	BlockUntilMessageCanBeSent();
#if VERBOSE
	std::cerr << "Sending message now" << std::endl;
#endif
	int sent_bytes = send(socket_handler_.get_handler(), message.c_str(), message.size(), 0);

	if(sent_bytes != (int)message.size()) {
		std::cerr << "Warning: Cannot sent message properly: " << message << std::endl;
		std::cerr << "errno is: " << ::strerror(errno) << std::endl;
		std::cerr << sent_bytes << " byte sent from " <<
			message.size() << ". Closing connection." << std::endl;
		socket_handler_.invalidate();
	}
}

boost::optional<std::vector<std::string>> Client::CheckForMessage() {
	std::vector<std::string> result;
	std::string buffer;

	std::stringstream consumer(received_buffer_);
	while(std::getline(consumer, buffer)) {
		if(buffer == ".") {
			auto position = consumer.tellg();
			if (position != std::stringstream::pos_type{-1}) {
				received_buffer_ = consumer.str().substr(position);
			} else {
				received_buffer_.clear();
			}
			return result;
		} else if(!buffer.empty()) {
			result.push_back(buffer);
		}
	}

	int buf_size = 4096;
	buffer = std::string(buf_size, '\0');

	int received_bytes = recv(socket_handler_.get_handler(), &buffer[0], buf_size, 0);

	switch(received_bytes) {
	case -1: {
		auto error = errno;
		if (error == EAGAIN || error == EWOULDBLOCK) {
			return boost::none;
		}
		std::cerr << "Error: recv failed due to something other than blocking!"
				<< std::endl;
	}
	case 0:
		std::cerr << "Connection closed." << std::endl;
		socket_handler_.invalidate();
		return std::vector<std::string>();
	}

	received_buffer_ += buffer.c_str();
	return CheckForMessage();
}

std::vector<std::string> Client::ReceiveMessage() {
	auto messages = CheckForMessage();
	if (messages) {
		return *messages;
	}
	BlockUntilMessageArrives();
	messages = CheckForMessage();
	assert(messages && "Block released yet messages not ready");
	return *messages;
}

void Client::BlockUntilMessageArrives() {
	::fd_set fds;
	FD_ZERO(&fds);
	FD_SET(socket_handler_.get_handler(), &fds);
	auto rc = ::select(socket_handler_.get_handler() + 1,
			&fds, nullptr, nullptr, nullptr);
	if (rc == EINTR) {
		std::cerr << "Interrupted system call" << std::endl;
		assert(false && "I haven't figured out what to do heere");
	}
	assert(rc > 0 && "Nothing apart from EINTR should come");
}

void Client::BlockUntilMessageCanBeSent() {
	::fd_set fds;
	FD_ZERO(&fds);
	FD_SET(socket_handler_.get_handler(), &fds);
	auto rc = ::select(socket_handler_.get_handler() + 1,
			nullptr, &fds, nullptr, nullptr);
	if (rc == EINTR) {
		std::cerr << "Interrupted system call" << std::endl;
		assert(false && "I haven't figured out what to do heere");
	}
	assert(rc > 0 && "Nothing apart from EINTR should come");
}

void Client::Init(const std::vector<std::string>& info_lines, Solver& solver) {
#if VERBOSE
	std::cerr << "We got these field informations:" << std::endl;
	for (auto& line : info_lines) {
		std::cerr << line << std::endl;
	}
#endif

	SaveInput(info_lines);

	auto info = parser_.ParseInit(info_lines);
	solver.Init(info.player);
}

bool Client::Process(const std::vector<std::string>& info_lines, Solver& solver) {
	SaveInput(info_lines);

	auto info = parser_.ParseTurn(info_lines);

	if (info.end) {
		std::cerr << "We got the end message: " << info_lines[0] << std::endl;
		solver.Shutdown();
		return false;
	}
#if VERBOSE
	std::cerr << "We got these tick informations:" << std::endl;
	for (auto& line : info_lines) {
		std::cerr << line << std::endl;
	}
#endif
	grid_ = info.grid;
	opponent_ = info.opponent;

	if (opponent_) {
		solver.Update(grid_, info.player);
	} else {
		wait_ = true;
		solver.Turn(grid_, info.player, info.target, info.extra,
			[&](const Response& response) {
				wait_ = false;
				response_ = response;
			});
	}
	return true;
}

void Client::SaveInput(const std::vector<std::string>& lines) {
	if (output_.is_open()) {
		for (const auto& line : lines) {
			output_ << line << std::endl;
		}
		output_ << ".\n";
	}
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

	bool firstUpdateReady = false;

	while (socket_handler_.valid()) {
		boost::optional<std::vector<std::string>> input = CheckForMessage();
		if (!input && firstUpdateReady) {
			solver.Idle();
		} else if (input && socket_handler_.valid()) {
			firstUpdateReady = true;
			if (!Process(*input, solver)) {
				break;
			}

			if (opponent_) {
				continue;
			}

			while (wait_) {
				solver.Idle();
			}

			if (socket_handler_.valid()) {
				if (input) {
					// response_ only set if we read something and processed it
					msg = FromResponse(response_);
					SendMessages(msg);
				}
			}
		}
	}
	std::cerr << "Game over" << std::endl;
}
