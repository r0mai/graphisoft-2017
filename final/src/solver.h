#ifndef SOLVER_H_INCLUDED
#define SOLVER_H_INCLUDED

#include <vector>
#include <string>

class solver {
	
public:
	void start(const std::vector<std::string>& start_infos);

	void init(const std::vector<std::string>& field_infos);

	std::vector<std::string> process(const std::vector<std::string>& tick_infos);

	// return value: Do you want to continue playing? if m == 1 then definitely
	bool after(const std::vector<std::string>& score_infos);

	void end(const std::string& message);
};

#endif // SOLVER_H_INCLUDED
