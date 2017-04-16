#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <cassert>

#include "Grid.h"
#include "Point.h"


namespace server {
namespace asio = boost::asio;

enum class Command {
	Login, Push, Goto, Over, Level, Message, Size, Displays, Player, MaxTick,
	Tick, Fields, End, Display, Position, Target, ExtraField
};

std::istream& operator>>(std::istream& is, Command& command) {
	std::string token;
	is >> token;
	if (token == "LOGIN") {
		command = Command::Login;
	} else if (token == "PUSH") {
		command = Command::Push;
	} else if (token == ".") {
		command = Command::Over;
	} else if (token == "LEVEL") {
		command = Command::Level;
	} else if (token == "MESSAGE") {
		command = Command::Message;
	} else if (token == "SIZE") {
		command = Command::Size;
	} else if (token == "DISPLAYS") {
		command = Command::Displays;
	} else if (token == "PLAYER") {
		command = Command::Player;
	} else if (token == "MAXTICK") {
		command = Command::MaxTick;
	} else if (token == "TICK") {
		command = Command::Tick;
	} else if (token == "FIELDS") {
		command = Command::Fields;
	} else if (token == "END") {
		command = Command::End;
	} else if (token == "DISPLAY") {
		command = Command::Display;
	} else if (token == "POSITION") {
		command = Command::Position;
	} else if (token == "TARGET") {
		command = Command::Target;
	} else if (token == "EXTRAFIELD") {
		command = Command::ExtraField;
	} else {
		std::cerr << "Failed parse: '" << token << "'" << std::endl;
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
		case Command::Over:
			return os << ".";
		case Command::Level:
			return os << "LEVEL";
		case Command::Message:
			return os << "MESSAGE";
		case Command::Size:
			return os << "SIZE";
		case Command::Displays:
			return os << "DISPLAYS";
		case Command::Player:
			return os << "PLAYER";
		case Command::MaxTick:
			return os << "MAXTICK";
		case Command::Tick:
			return os << "TICK";
		case Command::Fields:
			return os << "FIELDS";
		case Command::End:
			return os << "END";
		case Command::Display:
			return os << "DISPLAY";
		case Command::Position:
			return os << "POSITION";
		case Command::Target:
			return os << "TARGET";
		case Command::ExtraField:
			return os << "EXTRAFIELD";
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

	void setTeamName(std::string teamName) {
		this->teamName = std::move(teamName);
	}

	const std::string& getTeamName() const {
		return teamName;
	}


	int getScore() const {
		return 0;
	}

	Point getPosition() const {
		// TODO: implement
		return {0, 0};
	}

	int getTarget() const {
		// TODO: implement
		return 0;
	}

	int getExtraField() const {
		// TODO: implement
		return 15;
	}

	template<typename ArgumentType>
	Message<ArgumentType> readMessage(asio::yield_context yield) {
		auto line = readLine(yield);
		Message<ArgumentType> message{line};
		return message;
	}


	template<typename ArgumentType>
	void writeMessage(const Message<ArgumentType>& message,
			asio::yield_context yield) {
		auto line = boost::lexical_cast<std::string>(message.getCommand());
		for (const auto& argument: message.getArguments()) {
			line += " " + boost::lexical_cast<std::string>(argument);
		}
		line += "\n";
		socket.async_write_some(boost::asio::buffer(line), yield);
	}

private:
	std::string readLine(asio::yield_context yield) {
		std::string result = previousRead;
		auto newLinePos = result.find('\n');
		if (newLinePos != std::string::npos) {
			std::string untilLineEnd{previousRead, 0, newLinePos};
			previousRead = std::string{previousRead, newLinePos};
			return untilLineEnd;
		}
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
				if (newLinePos == currentChunk.size()) {
					previousRead = "";
				} else {
					previousRead = currentChunk.substr(newLinePos+1);
				}
				result += untilLineEnd;
				break;
			}
		}
		return result;
	}


	boost::asio::ip::tcp::socket socket;
	std::string teamName;
	std::string previousRead;
};


class Game {
public:
	Game() {
		grid.Init(6, 7, 4, 4);
		grid.Randomize();
	}

	void run() {
		using std::placeholders::_1;
		boost::asio::spawn(ioService,
				std::bind(&Game::start, this, _1));
		ioService.run();
	}

private:
	void handleNewClient(asio::ip::tcp::socket socket, asio::yield_context yield) {
		auto newPlayerId = playerCount++;
		auto p = players.emplace(newPlayerId, Client{std::move(socket)});
		auto& client = p.first->second;
		processLogin(client, yield);
		std::cerr << "After login" << std::endl;
	}

	void processLogin(Client& client, asio::yield_context yield) {
		auto loginCommand = client.readMessage<std::string>(yield);
		auto arguments = loginCommand.getArguments();
		client.setTeamName(arguments[0]);
		auto password = arguments[1];
		std::cerr << "Team " << client.getTeamName() << " logged in"
				<< std::endl;
		auto nextCommand = client.readMessage<int>(yield);
		int requestedLevel = 1;
		if (nextCommand.getCommand() == Command::Over) {
			std::cerr << "Client doesn't request any specific level"
					<< std::endl;
		} else {
			requestedLevel = nextCommand.getArguments()[0];
			std::cerr << "Client requests level: "
					<< requestedLevel << std::endl;
			auto overCommand = client.readMessage<int>(yield);
			assert(overCommand.getCommand() == Command::Over);
		}
		std::cerr << "Got All Info" << std::endl;

		client.writeMessage(
				Message<std::string>(Command::Message, {"OK"}), yield);
		client.writeMessage(
				Message<int>(Command::Level, {requestedLevel}), yield);
		client.writeMessage(
				Message<int>(Command::Size, {grid.Width(), grid.Height()}),
				yield);
		client.writeMessage(
				Message<int>(Command::Displays, {grid.DisplayCount()}), yield);
		client.writeMessage(
				Message<int>(Command::Player, {getId(client)}), yield);
		client.writeMessage(
				Message<int>(Command::MaxTick, {maxTicks}), yield);
		client.writeMessage(Message<int>(Command::Over, {}), yield);
		std::cerr << "Client all set up" << std::endl;
	}

	int getId(const Client& client) const {
		for (const auto& player: players) {
			if (&player.second == &client) {
				return player.first;
			}
		}
		return -1;
	}

	void processMove(Client& client, asio::yield_context yield) {
		auto pushCommand = client.readMessage<int>(yield);
		auto gotoCommand = client.readMessage<int>(yield);
		auto overCommand = client.readMessage<int>(yield);
	}

	void start(asio::yield_context yield) {
		asio::ip::tcp::acceptor acceptor{ioService,
				asio::ip::tcp::endpoint{asio::ip::tcp::v4(), 42500}};
		for (;;) {
			boost::system::error_code ec;
			asio::ip::tcp::socket socket{ioService};
			acceptor.async_accept(socket, yield[ec]);
			handleNewClient(std::move(socket), yield);
			// TODO: Something more sophisticated for starting th match
			break;
		}
		startMatch(yield);
	}

	void startMatch(asio::yield_context yield) {
		std::cerr << "All clients logged in, starting the round" << std::endl;
		for (; currentTick < maxTicks; ++currentTick) {
			std::cerr << "Tick: " << currentTick << std::endl;
			updateClients(yield);
		}
		for (auto& player: players) {
			auto& client = player.second;
			client.writeMessage(
					Message<int>(Command::End, {client.getScore()}), yield);
		}
	}

	void updateClients(asio::yield_context yield) {
		// Consider using a parallel for
		for (auto& player: players) {
			updateClient(player.second, yield);
		}
	}

	void updateClient(Client& client, asio::yield_context yield) {
		client.writeMessage(Message<int>(Command::Tick, {currentTick}), yield);
		const auto& fields = grid.Fields().GetFields();
		std::vector<int> intFields;
		intFields.reserve(fields.size());
		std::transform(fields.begin(), fields.end(),
				std::back_inserter(intFields),
				[](const Field& field) { return static_cast<int>(field); });
		client.writeMessage(Message<int>(Command::Fields, intFields), yield);

		const auto& displays = grid.Displays();
		for (std::size_t i=0; i<displays.size(); ++i) {
			const auto& display = displays[i];
			client.writeMessage(
					Message<int>(Command::Display,
							{static_cast<int>(i), display.x, display.y}),
					yield);
		}

		for (const auto& player: players) {
			int playerId = player.first;
			const auto& position = player.second.getPosition();
			client.writeMessage(
					Message<int>(Command::Position,
							{playerId, position.x, position.y}), yield);
		}

		client.writeMessage(
				Message<int>(Command::Player, {getId(client)}), yield);
		// TODO: What does this do?
		client.writeMessage(
				Message<std::string>(Command::Message, {"OK"}), yield);
		client.writeMessage(
				Message<int>(Command::Target, {client.getTarget()}), yield);
		client.writeMessage(Message<int>(Command::Over, {}), yield);
	}

	boost::asio::io_service ioService;
	Grid grid;
	std::size_t playerCount = 0;
	std::map<std::size_t, Client> players;
	int maxTicks = 1;
	int currentTick = 0;
};


} // namespace server

int main() {
	server::Game game;
	game.run();
}
