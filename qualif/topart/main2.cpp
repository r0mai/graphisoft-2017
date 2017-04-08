#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <numeric>
#include <tuple>


struct Ferry {
	Ferry() = default;
	Ferry(int src, int dst, int saving)
		: src(src), dst(dst), saving(saving)
	{}

	int src = 0;
	int dst = 0;
	int saving = 0;
};


std::ostream& operator<<(std::ostream& stream, const Ferry& ff) {
	stream << "(" << ff.src << " -> " << ff.dst << ") " << ff.saving;
	return stream;
}


class Lake {
public:
	void fromStream(std::istream& in);
	void ferriesToStream(std::ostream& out);

	int getOvertime() const { return overtime_; }

private:
	void calculateAllowed();

	std::vector<std::string> names_;
	std::vector<int> road_;
	std::vector<Ferry> ferry_;
	std::vector<std::vector<int>> allowed_;

	int time_ = 0;
	int overtime_ = 0;
};


void Lake::fromStream(std::istream& in) {
	std::map<std::string, int> name_map;
	std::vector<int> sums; // sums[i] : 0 -> i

	int n, sum;
	in >> n;

	for (int i = 0; i < n; ++i) {
		std::string name;
		in >> name;
		names_.push_back(name);
		name_map[name] = i;
	}

	sum = 0;
	sums.push_back(sum);
	for (int i = 0; i < n; ++i) {
		int d;
		in >> d;
		road_.push_back(d);
		sum += d;
		sums.push_back(sum);
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

		int saving = sums[q] - sums[p] - d;
		if (saving > 0) {
			// dont care about stupid ferries
			ferry_.emplace_back(p, q, saving);
		}
	}

	std::sort(ferry_.begin(), ferry_.end(),
		[](const Ferry& lhs, const Ferry& rhs) {
			return std::tie(lhs.src, lhs.dst) < std::tie(rhs.src, rhs.dst);
		});

	in >> time_;
	overtime_ = std::max(sums[n] - time_, 0);

	calculateAllowed();
}


void Lake::calculateAllowed() {
	auto fs = ferry_.size();
	allowed_.resize(fs);

	for (int i = 0; i < fs; ++i) {
		auto& vec = allowed_[i];
		int dst = ferry_[i].dst;
		for (int j = i + 1; j < fs; ++j) {
			if (ferry_[j].src >= dst) {
				vec.push_back(j);
			}
		}
	}
}


void Lake::ferriesToStream(std::ostream& out) {
	for (const auto& x : ferry_) {
		out << x << std::endl;
	}

	out << "Time we need to save = " <<  overtime_ << std::endl;
}


int main() {
	Lake lake;
	lake.fromStream(std::cin);
	lake.ferriesToStream(std::cerr);

	return 0;
}
