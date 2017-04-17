#include "EagerTaxicab.h"
#include "Util.h"

ClientResponse EagerTaxicab::GetResponse() {
	ClientResponse response;
	response.push.direction = {-1, 0};
	response.push.field = RotateLeft(extra_);
	grid_.Push(response.push.direction, response.push.field);
	response.move.target = {};

	return response;
}
