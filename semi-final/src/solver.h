#ifndef SOLVER_H_INCLUDED
#define SOLVER_H_INCLUDED

#include <vector>
#include <string>
#include "Point.h"
#include "Grid.h"

class solver {
public:
	void init(const std::vector<std::string>& field_infos);

	std::vector<std::string> process(const std::vector<std::string>& tick_infos);

	void end(const std::string& message);
private:
	// stuff we get form init()
	int level_ = -1; // index of the map
	int max_tick_ = -1;

	Grid grid_;
	int player_index_ = -1;

	// stuff from process()
	int current_tick_ = -1;
	int current_player_ = -1;
	int target_display_ = -1;
	int extra_field_ = -1;
};

bool NoPositiveXBorder(int type);
bool NoNegativeXBorder(int type);
bool NoPositiveYBorder(int type);
bool NoNegativeYBorder(int type);

#if 0
template<typename T, typename U>
Matrix<T> CloneMatrixDimensions(const Matrix<U>& m, const T& default_value = T{}) {
	Matrix<T> result{m.size()};
	for (int i = 0; i < m.size(); ++i) {
		result[i].resize(m[i].size(), default_value);
	}
	return result;
}

Matrix<bool> FloodFill(const Matrix<int>& field, const Point& origin);
#endif

#endif // SOLVER_H_INCLUDED
