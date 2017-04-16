#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>


namespace server {
namespace asio = boost::asio;

enum class Command {
	Login, Push, Goto
};

std::istream& operator>>(std::istream& is, Command& command) {
	std::string token;
	is >> token;
	if (token == "LOGIN") {
		command = Command::Login;
	} else if (token == "PUSH") {
		command = Command::Push;
	} else {
		throw std::runtime_error("Could not parse input");
	}
	return is;
}


std::ostream& operator<<(std::ostream& os, const Command& command) {
	switch (command) {
		case Command::Login:
			return os << "LOGIN";
		case Command::Push:
			return os << "PUSH";
		case Command::Goto:
			return os << "GOTO";
	}
}


template<typename ArgumentType>
class Message {
public:
	Message(Command command, std::vector<ArgumentType> arguments) :
		command(command), arguments(std::move(arguments)) { }

	explicit Message(const std::string& line) {
		std::stringstream lineStream(line);
		lineStream >> command;
		while (lineStream.good()) {
			ArgumentType value;
			lineStream >> value;
			arguments.push_back(std::move(value));
		}
	}

	Command getCommand() const { return command; }
	const std::vector<ArgumentType>& getArguments() const { return arguments; }


private:
	Command command;
	std::vector<ArgumentType> arguments;
};


class Client {
public:
	explicit Client(asio::ip::tcp::socket socket) :
		socket(std::move(socket)) {
		std::cerr << "Client logged in from: "
				<< this->socket.remote_endpoint() << std::endl;
	}

	void processLogin(asio::yield_context yield) {
		auto loginCommand = readMessage<std::string>(yield);
		auto arguments = loginCommand.getArguments();
		teamName = arguments[0];
		auto password = arguments[1];
		std::cerr << "Team " << teamName << " logged in" << std::endl;
	}

	const std::string& getTeamName() const {
		return teamName;
	}

private:
	std::string readLine(asio::yield_context yield) {
		std::string result = previousRead;
		for (;;) {
			char buffer[128];
			std::size_t n = socket.async_read_some(boost::asio::buffer(buffer),
					yield);
			std::string currentChunk{buffer, buffer+n};
			auto newLinePos = currentChunk.find('\n');
			if (newLinePos == std::string::npos) {
				result += currentChunk;
				// Keep reading
			} else {
				std::string untilLineEnd{currentChunk, 0, newLinePos};
				previousRead = std::string{currentChunk, newLinePos};
				result += untilLineEnd;
				break;
			}
		}
		return result;
	}


	template<typename ArgumentType>
	Message<ArgumentType> readMessage(asio::yield_context yield) {
		auto line = readLine(yield);
		Message<ArgumentType> message{line};
		return message;
	}


	boost::asio::ip::tcp::socket socket;
	std::string teamName;
	std::string previousRead;
};


void handleClient(asio::ip::tcp::socket socket, asio::yield_context yield) {
	Client client{std::move(socket)};
	client.processLogin(yield);
}


void start(asio::io_service& ioService, asio::yield_context yield) {
	asio::ip::tcp::acceptor acceptor{ioService,
			asio::ip::tcp::endpoint{asio::ip::tcp::v4(), 42500}};
	for (;;) {
		boost::system::error_code ec;
		asio::ip::tcp::socket socket{ioService};
		acceptor.async_accept(socket, yield[ec]);
		boost::asio::spawn(ioService,
				[&socket](asio::yield_context yield) {
					handleClient(std::move(socket), yield); } );
	}
}

} // namespace server

int main() {
	boost::asio::io_service ioService;
	boost::asio::spawn(ioService, [&](boost::asio::yield_context yield) {
			server::start(ioService, yield); });
	ioService.run();
}
