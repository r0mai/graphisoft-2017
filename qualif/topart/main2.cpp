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


class Lake {
public:
	void fromStream(std::istream& in);

	// // If we use first, then we can't use any of second
	// std::map<std::pair<int, int>, std::map<int, int>>
	// 		getConflictingFerries() const;

	// int getRoadLength() const;
	// int getTimeBudget() const;

private:
	std::vector<std::string> names_;
	std::vector<int> road_;
	std::vector<Ferry> ferry_;
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

#if 1
	for (const auto& x : ferry_) {
		std::cerr << x.src << " " << x.dst << " " << x.saving << std::endl;
	}

	std::cerr << overtime_ << std::endl;
#endif
}


// std::map<std::pair<int, int>, std::map<int, int>>
// Lake::getConflictingFerries() const {
// 	std::map<std::pair<int, int>, std::map<int, int>> result;
// 	for (auto it = ferry_.begin(); it != ferry_.end(); ++it) {
// 		const auto& villageFerries = *it;
// 		const auto& src = villageFerries.first;
// 		const auto& availableFerries = villageFerries.second;
// 		for (const auto& selectedFerry: availableFerries) {
// 			const auto& dst = selectedFerry.first;
// 			auto selectedResult = std::map<int, int>{};

// 			// Filter ferries starting before src, but finishing before dst
// 			for (auto it_ = ferry_.begin(); it_ != it; ++it_) {
// 				for (const auto& possibleFerry: it->second) {
// 					if (possibleFerry.first <= dst) {
// 						selectedResult[it_->first] = possibleFerry.first;
// 					}
// 				}
// 			}

// 			// Filter ferries starting after src, but before dst
// 			const auto firstFerryAfterRoute = ferry_.upper_bound(dst);
// 			for (auto it_ = it; it_ != firstFerryAfterRoute; ++it_) {
// 				for(const auto& possibleFerry: it_->second) {
// 					selectedResult[it_->first] = possibleFerry.first;
// 				}
// 			}

// 			result[std::make_pair(src, dst)] = std::move(selectedResult);

// 		}
// 	}
// 	return result;
// }


// int Lake::getRoadLength() const {
// 	return std::accumulate(road_.begin(), road_.end(), 0);
// }


// int Lake::getTimeBudget() const {
// 	return time_;
// }


// void conflictingFerriesToStream(std::ostream& os,
// 		const std::map<std::pair<int, int>, std::map<int, int>>& conflicts) {
// 	for (const auto& conflict: conflicts) {
// 		const auto& ferry = conflict.first;
// 		os << ferry.first << "->" << ferry.second << "\n{\n";
// 		for (const auto& conflictingFerry: conflict.second) {
// 			os << "    " << conflictingFerry.first << "->"
// 					<< conflictingFerry.second << "\n";
// 		}
// 		os << "}\n";
// 	}
// }


int main() {
	Lake lake;
	lake.fromStream(std::cin);
	// const auto& conflicts = lake.getConflictingFerries();
	// conflictingFerriesToStream(std::cerr, conflicts);

	// const auto& roadLength = lake.getRoadLength();
	// const auto& budget = lake.getTimeBudget();
	// auto excessTime = std::max(roadLength - budget, 0);
	// std::cerr << "Road length is: " << roadLength << ", budget: " << budget
	// 		<< ", time we need to save: " << excessTime << std::endl;

}
