#include "EagerTaxicab.h"

ClientResponse EagerTaxicab::GetResponse() {
	ClientResponse response;
	response.push.direction = {-1, 0};
	response.push.field = extra_;
	grid_.Push(response.push.direction, response.push.field);

	response.move.target = grid_.Positions()[player_];

	return response;
}
