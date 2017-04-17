#include "AI.h"

AI::AI(const Grid& grid, int player, int target_display, Field extra)
	: grid_(grid)
	, player_(player)
	, target_display_(target_display)
	, extra_(extra)
{}
