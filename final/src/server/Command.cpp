#include "Command.hpp"

namespace server {

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

} // namespace server
