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

#include "server/Command.hpp"


namespace server {
namespace asio = boost::asio;


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
	Client(asio::ip::tcp::socket socket, int id) :
		id(id), socket(std::move(socket)) {
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
		return score;
	}

	void onHitTarget() {
		++score;
		++target;
	}

	Point getPosition(Grid& grid) const {
		return grid.Positions()[id];
	}

	void moveTo(Grid& grid, int x, int y) {
		grid.UpdatePosition(id, Point{x, y});
	}

	int getTarget() const {
		// TODO: coordinate with other players, or just use a random int
		return target;
	}

	int getId() const {
		return id;
	}

	Field getExtraField() const {
		return field;
	}

	void setExtraField(Field field) {
		this->field = field;
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


	int id;
	boost::asio::ip::tcp::socket socket;
	Field field = static_cast<Field>(15);
	std::string teamName;
	int score = 0;
	int target = 0;
	std::string previousRead;
};


class Game {
public:
	Game(int maxPlayers, int maxTicks) :
		maxPlayers(maxPlayers), maxTicks(maxTicks)
	{
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
		auto p = players.emplace(
				newPlayerId, Client{std::move(socket), newPlayerId});
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
				Message<int>(Command::Player, {client.getId()}), yield);
		client.writeMessage(
				Message<int>(Command::MaxTick, {maxTicks}), yield);
		client.writeMessage(Message<int>(Command::Over, {}), yield);
		std::cerr << "Client all set up" << std::endl;
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
			const auto& position = player.second.getPosition(grid);
			client.writeMessage(
					Message<int>(Command::Position,
							{playerId, position.x, position.y}), yield);
		}

		client.writeMessage(
				Message<int>(Command::Player, {client.getId()}), yield);
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
			auto popped = grid.Push(arguments[0], arguments[1], arguments[2],
					static_cast<Field>(arguments[3]));
			client.setExtraField(popped);
		}
		if (gotoMessage) {
			const auto& arguments = gotoMessage->getArguments();
			client.moveTo(grid, arguments[0], arguments[1]);
			const auto& target = grid.Displays()[client.getTarget()];
			if (client.getPosition(grid) == target) {
				client.onHitTarget();
			}
		}
	}

	boost::asio::io_service ioService;
	Grid grid;
	int playerCount = 0;
	int maxPlayers;
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
			("players", po::value<int>(), "players to wait for before starting, defaults to 4")
			("ticks", po::value<int>(), "ticks to run the game for, defaults to 10");
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
	int playersToWaitFor = 4;
	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return 0;
	}
	if (vm.count("players")) {
		playersToWaitFor = vm["players"].as<int>();
	}
	int maxTicks = 10;
	if (vm.count("ticks")) {
		maxTicks = vm["ticks"].as<int>();
	}
	server::Game game{playersToWaitFor, maxTicks};
	game.run();
}
