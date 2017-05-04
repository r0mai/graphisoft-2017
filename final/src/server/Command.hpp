#pragma once

#include <iostream>
#include <map>
#include <cassert>

namespace server {

enum class Command {
	Login, Push, Goto, Over, Level, Message, Size, Displays, Player, MaxTick,
	Tick, Fields, End, Display, Position, Target, ExtraField
};

std::istream& operator>>(std::istream& is, Command& command);
std::ostream& operator<<(std::ostream& os, const Command& command);

} // namespace server
