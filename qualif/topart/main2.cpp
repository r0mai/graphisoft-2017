#include <iostream>
#include <map>
#include <vector>


class Lake {
public:
	void fromStream(std::istream& in);

private:
	std::vector<std::string> names_;
	std::vector<int> road_;
	std::map<int, std::map<int, int>> ferry_;
	int time_ = 0;
};


void Lake::fromStream(std::istream& in) {
	std::map<std::string, int> name_map;

	int n;
	in >> n;

	for (int i = 0; i < n; ++i) {
		std::string name;
		in >> name;
		names_.push_back(name);
		name_map[name] = i;
	}

	for (int i = 0; i < n; ++i) {
		int d;
		in >> d;
		road_.push_back(d);
	}

	int m;
	in >> m;

	for (int i = 0; i < m; ++i) {
		int d;
		std::string src, dst;
		in >> src >> dst >> d;
		if (name_map.find(src) == name_map.end() ||
		 	name_map.find(dst) == name_map.end())
		{
			// bad input
			continue;
		}

		int p = name_map[src];
		int q = name_map[dst];
		if (q == 0) {
			q = n;
		}
		ferry_[p][q] = d;
	}

	in >> time_;
}


int main() {
	Lake lake;
	lake.fromStream(std::cin);
}
