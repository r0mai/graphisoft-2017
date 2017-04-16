#ifndef SOLVER_H_INCLUDED
#define SOLVER_H_INCLUDED

#include <vector>
#include <string>
#include "Point.h"

template<typename T>
using Matrix = std::vector<std::vector<T>>;

class solver {
public:
	void init(const std::vector<std::string>& field_infos);

	std::vector<std::string> process(const std::vector<std::string>& tick_infos);

	void end(const std::string& message);
private:
	// stuff we get form init()
	int level_ = -1;
	int width_ = -1; // N
	int height_ = -1; // M
	int display_count_ = -1;
	int player_index_ = -1;
	int max_tick_ = -1;

	// stuff from process()
	struct {
		int tick = -1;
		int current_player = -1;
		int target_display = -1;
		int our_field_type = -1;
		Matrix<int> field;
		std::vector<Point> displays;
	} tick;
};

bool NoPositiveXBorder(int type);
bool NoNegativeXBorder(int type);
bool NoPositiveYBorder(int type);
bool NoNegativeYBorder(int type);

template<typename T, typename U>
Matrix<T> CloneMatrixDimensions(const Matrix<U>& m, const T& default_value = T{}) {
	Matrix<T> result{m.size()};
	for (int i = 0; i < m.size(); ++i) {
		result[i].resize(m[i].size(), default_value);
	}
	return result;
}

Matrix<bool> FloodFill(const Matrix<int>& field, const Point& origin);

#endif // SOLVER_H_INCLUDED
