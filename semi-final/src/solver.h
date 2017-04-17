#ifndef SOLVER_H_INCLUDED
#define SOLVER_H_INCLUDED

#include <vector>
#include <string>
#include "Point.h"
#include "Grid.h"
#include "FloodFill.h"
#include "ClientResponse.h"

class solver {
public:
	void init(const std::vector<std::string>& field_infos);

	std::vector<std::string> process(const std::vector<std::string>& tick_infos);

	void end(const std::string& message);
private:

	ClientResponse DoEagerTaxicab();

	std::vector<std::string> ClientResponseToStrings(
		const ClientResponse& response) const;

	// stuff we get form init()
	int level_ = -1; // index of the map
	int max_tick_ = -1;

	Grid grid_;
	int player_index_ = -1;

	// stuff from process()
	int current_tick_ = -1;
	int current_player_ = -1;
	int target_display_ = -1;
	Field extra_field_ = Field(-1);
};

#endif // SOLVER_H_INCLUDED
