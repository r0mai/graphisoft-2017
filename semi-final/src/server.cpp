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
	Message(Command command, std::vector<ArgumentType> arguments)
		: command(command)
		, arguments(std::move(arguments))
	{}

	explicit Message(const std::string& line) {
		std::stringstream lineStream(line);
		str = line;
		lineStream >> command;
		while (lineStream.good()) {
			ArgumentType value;
			lineStream >> value;
			arguments.push_back(std::move(value));
		}
	}

	const std::string& getString() const { return str; }
	Command getCommand() const { return command; }
	const std::vector<ArgumentType>& getArguments() const { return arguments; }


private:
	std::string str;
	Command command;
	std::vector<ArgumentType> arguments;
};


class Client {
public:
	Client(asio::ip::tcp::socket socket, int id) :
		id(id), socket(std::move(socket)) {
		std::cerr << "Client logged in from: "
				<< this->socket.remote_endpoint() << std::endl;
		asio::ip::tcp::no_delay option{true};
		this->socket.set_option(option);
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
	}

	void onTargetInvalidated() {
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
	void addMessage(const Message<ArgumentType>& message) {
		auto line = boost::lexical_cast<std::string>(message.getCommand());
		for (const auto& argument: message.getArguments()) {
			line += " " + boost::lexical_cast<std::string>(argument);
		}
		line += "\n";
		outBuffer += line;
	}

	void sendMessages(asio::yield_context yield) {
		socket.async_write_some(boost::asio::buffer(outBuffer), yield);
		outBuffer.clear();
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
	std::string outBuffer;
};


class Game {
public:
	Game(int maxPlayers, int maxTicks) :
		maxPlayers(maxPlayers), maxTicks(maxTicks)
	{
		grid.Init(6, 7, 4, maxPlayers);
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

		client.addMessage(Message<std::string>(Command::Message, {"OK"}));
		client.addMessage(Message<int>(Command::Level, {requestedLevel}));
		client.addMessage(
				Message<int>(Command::Size, {grid.Width(), grid.Height()}));
		client.addMessage(
				Message<int>(Command::Displays, {grid.DisplayCount()}));
		client.addMessage(Message<int>(Command::Player, {client.getId()}));
		client.addMessage(Message<int>(Command::MaxTick, {maxTicks}));
		client.addMessage(Message<int>(Command::Over, {}));
		client.sendMessages(yield);
		std::cerr << "Client all set up" << std::endl;
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
			const auto& displays = grid.Displays();
			std::size_t displaysAvailable =
					std::count_if(displays.begin(), displays.end(),
							&IsValid);
			if (displaysAvailable == 0) {
				std::cerr << "Ending match due to all displays being collected"
						<< std::endl;
				break;
			}
			runTick(yield);
		}
		for (auto& player: players) {
			auto& client = player.second;
			std::cout << "Team: " << client.getTeamName() << " scored: "
					<< client.getScore() << std::endl;
			client.addMessage(Message<int>(Command::End, {client.getScore()}));
			client.sendMessages(yield);
		}
	}

	void runTick(asio::yield_context yield) {
		for (int currentPlayer=0; currentPlayer<playerCount; ++currentPlayer) {
			std::cerr << "Current player is: " << players.at(currentPlayer).getTeamName() << std::endl;
			std::cerr << "Informing them, and awaiting there move" << std::endl;
			std::cout << grid << std::endl;
			updateCurrentClient(players.at(currentPlayer), yield);
			for(int i=0; i<playerCount; ++i) {
				if (i == currentPlayer) {
					continue;
				}
				std::cerr << "    updating other team: " << players.at(i).getTeamName() << std::endl;
				updateClient(players.at(i), currentPlayer, true, yield);
			}
			evaluateClientInstruction(players.at(currentPlayer), yield);
		}
	}

	void updateClient(Client& client, int originatingPlayer, bool sendOver,
			asio::yield_context yield) {
		client.addMessage(Message<int>(Command::Tick, {currentTick}));
		const auto& fields = grid.Fields().GetFields();
		std::vector<int> intFields;
		intFields.reserve(fields.size());
		std::transform(fields.begin(), fields.end(),
				std::back_inserter(intFields),
				[](const Field& field) { return static_cast<int>(field); });
		client.addMessage(Message<int>(Command::Fields, intFields));

		const auto& displays = grid.Displays();
		for (std::size_t i=0; i<displays.size(); ++i) {
			const auto& display = displays[i];
			if (!IsValid(display)) {
				continue;
			}
			client.addMessage(Message<int>(Command::Display,
					{static_cast<int>(i), display.x, display.y}));
		}

		for (const auto& player: players) {
			int playerId = player.first;
			const auto& position = player.second.getPosition(grid);
			client.addMessage(
					Message<int>(Command::Position,
							{playerId, position.x, position.y}));
		}
		client.addMessage(
				Message<int>(Command::Player, {originatingPlayer}));
		if (sendOver) {
			client.addMessage(Message<int>(Command::Over, {}));
			client.sendMessages(yield);
		}
	}

	void updateCurrentClient(Client& client, asio::yield_context yield) {
		// TODO: What does this do?
		updateClient(client, client.getId(), false, yield);
		client.addMessage(Message<std::string>(Command::Message, {"OK"}));
		client.addMessage(Message<int>(Command::Target, {client.getTarget()}));
		client.addMessage(
				Message<int>(Command::ExtraField, {client.getExtraField()}));
		client.addMessage(Message<int>(Command::Over, {}));
		client.sendMessages(yield);
	}

	void evaluateClientInstruction(Client& client, asio::yield_context yield) {
		std::cerr << "Running instructions of: "
				<< client.getTeamName() << std::endl;
		boost::optional<Message<int>> pushMessage;
		boost::optional<Message<int>> gotoMessage;
		auto message = client.readMessage<int>(yield);
		while (message.getCommand() != Command::Over) {
			std::cerr << "Read command: " << message.getString() << std::endl;
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
			Point target = grid.Displays()[client.getTarget()];
			if (client.getPosition(grid) == target) {
				client.onHitTarget();
				for (auto& player : players) {
					if (grid.Displays()[player.second.getTarget()]
							== client.getPosition(grid)) {
						player.second.onTargetInvalidated();
					}
				}
				removeDisplay(target);
			}
		}
	}

	void removeDisplay(Point target) {
		for (int i = 0; i < grid.DisplayCount(); ++i) {
			const auto& display = grid.Displays()[i];
			if (display == target) {
				grid.UpdateDisplay(i, Point{});
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
	for (;;) {
		std::cerr << "Starting new game" << std::endl;
		server::Game game{playersToWaitFor, maxTicks};
		game.run();
	}
}
