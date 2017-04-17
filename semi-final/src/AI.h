#pragma once

#include "Grid.h"
#include "Point.h"
#include "Field.h"
#include "ClientResponse.h"

class AI {
public:
	AI(const Grid& grid, int player, int target_display, Field extra);

	virtual ClientResponse GetResponse() = 0;
protected:
	Grid grid_;
	int player_;
	int target_display_;
	Field extra_;
};
