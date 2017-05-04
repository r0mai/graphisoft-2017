#include "platform_dep.h"
#include "solver.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <stdexcept>

class client {
	platform_dep::tcp_socket socket_handler;
	std::string received_buffer;
	solver your_solver;
public:
	client(const char host_name[], unsigned short port, 
		const char team_name[], const char password[]) {

		if(!socket_handler.valid()) {
			throw std::runtime_error("Error: Cannot open a socket!");
		}

		hostent* host = gethostbyname(host_name);

		if(!host) {
			throw std::runtime_error("Error: Cannot find host: " + std::string(host_name));
		}
		
		sockaddr_in socket_address;
		socket_address.sin_family = AF_INET;
		socket_address.sin_port = htons(port);
		socket_address.sin_addr.s_addr = inet_addr(inet_ntoa(*(in_addr *)host->h_addr_list[0]));
		
		if(connect(socket_handler.get_handler(), (struct sockaddr*)&socket_address, 
			sizeof(socket_address)) != 0) {
			throw std::runtime_error("Error: Cannot connect to the server: " + 
				std::string(host_name) + ":" + std::to_string(port) + 
				" error code: " + std::to_string(socketerrno));
		}

		std::vector<std::string> login_messages;

		login_messages.push_back(std::string("LOGIN ") + team_name + " " + password);

		send_messages(login_messages);
	}

	void send_messages(const std::vector<std::string>& messages) {
		std::string message;

		for(std::size_t i = 0; i < messages.size(); ++i) {
			message += messages[i] + '\n';
		}
		message += ".\n";

		int sent_bytes = send(socket_handler.get_handler(), message.c_str(), message.size(), 0);

		if(sent_bytes != (int)message.size()) {
			std::cerr << "Warning: Cannot sent message properly: " << message << std::endl;
			std::cerr << sent_bytes << " byte sent from " <<
				message.size() << ". Closing connection." << std::endl;
			socket_handler.invalidate();
		}
	}

	std::vector<std::string> receive_message() {
		std::vector<std::string> result;
		std::string buffer;

		std::stringstream consumer(received_buffer);
		while(std::getline(consumer, buffer)) {
			if(buffer == ".") {
				if(consumer.tellg() == std::streampos(-1)) {
					received_buffer.clear();
				} else {
					received_buffer = consumer.str().substr(consumer.tellg());
				}
				return result;
			} else if(!buffer.empty()) {
				result.push_back(buffer);
			}
		}

		char array_buffer[512];

		int received_bytes = recv(socket_handler.get_handler(), array_buffer, 511, 0);
		
		switch(received_bytes) {
		case -1:
			std::cerr << "Error: recv failed!" << std::endl;
		case 0:
			std::cerr << "Connection closed." << std::endl;
			socket_handler.invalidate();
			return std::vector<std::string>();
		}
		array_buffer[received_bytes] = '\0';

		received_buffer += array_buffer;
		return receive_message();
	}
public:
	void run() {
		while(socket_handler.valid()) {
			std::vector<std::string> tmp = receive_message();
			if(socket_handler.valid()) {
				if(tmp.size() == 1 && tmp[0].find("END") == 0) {
					your_solver.end(tmp[0]);
					break;
				} else if(tmp.size() == 2 && tmp[0].find("MESSAGE") == 0) {
					your_solver.start(tmp);
				} else if(tmp.size() == 2 && tmp[0].find("SCORE") == 0) {
					if(!your_solver.after(tmp)) {
						break;
					}
				} else if(tmp.size() > 0 && tmp[0].find("ID") == 0) {
					your_solver.init(tmp);
				} else {
					tmp = your_solver.process(tmp);

					if(!tmp.empty()) {
						send_messages(tmp);
					}
				}
			}
		}
		std::cerr << "Game over" << std::endl;
	}
};

int main(int argc, char** argv) {
	/* config area */
	const char host_name[] = "SZERVERNEVE";
	const unsigned short port = 42500;
	const char team_name[] = "CSAPATNEVETEK";
	const char password[] = "JELSZAVATOK";
	
	try {
		platform_dep::enable_socket _;
	
		client(host_name, port, team_name, password).run();

	} catch(std::exception& e) {
		std::cerr << "Exception throwed. what(): " << e.what() << std::endl;
	}
}
