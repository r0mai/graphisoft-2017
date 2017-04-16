#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <cassert>

#include <boost/program_options.hpp>

#include "Grid.h"
#include "Point.h"


namespace server {
namespace asio = boost::asio;

enum class Command {
	Login, Push, Goto, Over, Level, Message, Size, Displays, Player, MaxTick,
	Tick, Fields, End, Display, Position, Target, ExtraField
};

namespace detail {

std::map<std::string, Command> getTokenToCommand() {
	std::map<std::string, Command> result;
	result.emplace("LOGIN", Command::Login);
	result.emplace("PUSH", Command::Push);
	result.emplace(".", Command::Over);
	result.emplace("LEVEL", Command::Level);
	result.emplace("MESSAGE", Command::Message);
	result.emplace("SIZE", Command::Size);
	result.emplace("DISPLAYS", Command::Displays);
	result.emplace("PLAYER", Command::Player);
	result.emplace("MAXTICK", Command::MaxTick);
	result.emplace("TICK", Command::Tick);
	result.emplace("FIELDS", Command::Fields);
	result.emplace("END", Command::End);
	result.emplace("DISPLAY", Command::Display);
	result.emplace("POSITION", Command::Position);
	result.emplace("TARGET", Command::Target);
	result.emplace("EXTRAFIELD", Command::ExtraField);
	result.emplace("GOTO", Command::Goto);

	return result;
}

std::map<Command, std::string> getCommandToToken() {
	std::map<Command, std::string> result;
	result.emplace(Command::Login, "LOGIN");
	result.emplace(Command::Push, "PUSH");
	result.emplace(Command::Over, ".");
	result.emplace(Command::Level, "LEVEL");
	result.emplace(Command::Message, "MESSAGE");
	result.emplace(Command::Size, "SIZE");
	result.emplace(Command::Displays, "DISPLAYS");
	result.emplace(Command::Player, "PLAYER");
	result.emplace(Command::MaxTick, "MAXTICK");
	result.emplace(Command::Tick, "TICK");
	result.emplace(Command::Fields, "FIELDS");
	result.emplace(Command::End, "END");
	result.emplace(Command::Display, "DISPLAY");
	result.emplace(Command::Position, "POSITION");
	result.emplace(Command::Target, "TARGET");
	result.emplace(Command::ExtraField, "EXTRAFIELD");
	result.emplace(Command::Goto, "GOTO");
	return result;
}

} // namespace detail

std::istream& operator>>(std::istream& is, Command& command) {
	std::string token;
	is >> token;

	static const auto tokenToCommand = detail::getTokenToCommand();

	auto it = tokenToCommand.find(token);
	if (it != tokenToCommand.end()) {
		command = it->second;
		return is;
	}
	std::cerr << "Failed parse: '" << token << "'" << std::endl;
	throw std::runtime_error("Could not parse input");
}


std::ostream& operator<<(std::ostream& os, const Command& command) {
	static const auto commandToToken = detail::getCommandToToken();
	auto it = commandToToken.find(command);
	assert(it != commandToToken.end());
	return os << it->second;
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
		return position;
	}

	void moveTo(int x, int y) {
		position = Point{x, y};
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
			if (newLinePos == previousRead.size()) {
				previousRead = "";
			} else {
				previousRead = previousRead.substr(newLinePos+1);
			}
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
	Point position;
};


class Game {
public:
	Game(unsigned maxPlayers) : maxPlayers(maxPlayers) {
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
			if (playerCount == maxPlayers) {
				break;
			}
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

		evaluateClientInstruction(client, yield);
	}

	void evaluateClientInstruction(Client& client, asio::yield_context yield) {
		std::cerr << "Running instructions of: "
				<< client.getTeamName() << std::endl;
		boost::optional<Message<int>> pushMessage;
		boost::optional<Message<int>> gotoMessage;
		auto message = client.readMessage<int>(yield);
		while (message.getCommand() != Command::Over) {
			std::cerr << "Read command" << std::endl;
			switch(message.getCommand()) {
				default:
					std::cerr << "Got invalid command from player: "
							<< message.getCommand() << std::endl;
					continue;
				case Command::Goto:
					if (!gotoMessage) {
						gotoMessage = message;
					}
				case Command::Push:
					if (!pushMessage) {
						pushMessage = message;
					}
					break;
			}
			message = client.readMessage<int>(yield);
		}
		// Commands read, now evaluate
		if (pushMessage) {
			const auto& arguments = pushMessage->getArguments();
			grid.Push(arguments[0], arguments[1], arguments[2],
					static_cast<Field>(arguments[3]));
		}
		if (gotoMessage) {
			const auto& arguments = gotoMessage->getArguments();
			client.moveTo(arguments[0], arguments[1]);
		}
	}

	boost::asio::io_service ioService;
	Grid grid;
	std::size_t playerCount = 0;
	unsigned maxPlayers;
	std::map<std::size_t, Client> players;
	int maxTicks = 1;
	int currentTick = 0;
};


} // namespace server

int main(int argc, const char** argv) {
	namespace po = boost::program_options;
	po::options_description desc{"Allowed Options"};
	desc.add_options()
			("help", "this help message")
			("players", po::value<unsigned>(), "players to wait for before starting, defaults to 4");
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
	unsigned playersToWaitFor = 4;
	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return 0;
	}
	if (vm.count("players")) {
		playersToWaitFor = vm["players"].as<unsigned>();
	}
	server::Game game{playersToWaitFor};
	game.run();
}
