#pragma once

#include "AI.h"

class EagerTaxicab : public AI {
public:
	using AI::AI;

	ClientResponse GetResponse() override;
};
