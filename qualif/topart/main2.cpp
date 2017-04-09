#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <numeric>
#include <tuple>
#include <chrono>


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


using Indices = std::vector<int>;
using Clock = std::chrono::steady_clock;
using Duration = std::chrono::duration<double>;

class Lake {
public:
	Lake();
	void fromStream(std::istream& in);
	void ferriesToStream(std::ostream& out);
	void allowedToStream(std::ostream& out);
	void statsToStream(std::ostream& out);
	void solve();
	void solutionToStream(std::ostream& out);

	int getOvertime() const { return overtime_; }

private:
	void calculateAllowed();
	bool recurse(const Indices& used, const Indices& remain, int saved);

	std::vector<std::string> names_;
	std::vector<int> road_;
	std::vector<Ferry> ferry_;
	std::vector<Indices> allowed_;

	Indices solution_;
	int best_ = 0;
	int optimize_count_ = 0;
	int solve_count_ = 0;

	int time_ = 0;
	int overtime_ = 0;
	int loop_ = 0;

	Clock::time_point start_time_;
};

Lake::Lake() {
	start_time_ = Clock::now();
}

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

	names_.push_back(names_[0]);

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

		if (p > q) {
			// going backwards, bad input
			continue;
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

	for (std::size_t i = 0; i < fs; ++i) {
		auto& vec = allowed_[i];
		int src = ferry_[i].src;
		int dst = ferry_[i].dst;

		for (std::size_t j = 0; j < i; ++j) {
			const auto& ferry = ferry_[j];
			if (ferry.dst <= src) {
				vec.push_back(j);
			}
		}

		// Index of ferries departing later than dst
		std::size_t dstIndex = i;
		for (; dstIndex < fs; ++dstIndex) {
			if (ferry_[dstIndex].src >= dst) {
				break;
			}
		}

		for (std::size_t j = dstIndex; j < fs; ++j) {
			vec.push_back(j);
		}
	}
}


void Lake::ferriesToStream(std::ostream& out) {
	int i = 0;
	for (const auto& x : ferry_) {
		out << i << ": " << x << std::endl;
		++i;
	}

	out << "Time we need to save = " <<  overtime_ << std::endl;
}


void Lake::allowedToStream(std::ostream& out) {
	for (std::size_t i = 0; i < ferry_.size(); ++i) {
		const auto& ferry = ferry_[i];
		const auto& allowedFerries = allowed_[i];
		out << ferry.src << "->" << ferry.dst << " {\n";
		for (const auto& allowedFerryIndex: allowedFerries) {
			const auto& allowedFerry = ferry_[allowedFerryIndex];
			out << "    " << allowedFerry.src << "->"
					<< allowedFerry.dst << "\n";
		}
		out << "}\n";
	}
}


bool Lake::recurse(const Indices& used, const Indices& remain, int saved) {
	if (--loop_ < 0) {
		loop_ = 10000;
		auto current_time = Clock::now();
		auto delta = std::chrono::duration_cast<Duration>(current_time - start_time_);

		// std::cerr << delta.count()
		// 	<< " " << solve_count_
		// 	<< " " << optimize_count_
		// 	<< " " << best_
		//  	<< std::endl;

		if (delta > Duration(9.0)) {
			return true;
		}
	}

	if (saved >= overtime_) {
		if (best_ < overtime_ || saved - overtime_ < best_ - overtime_) {
			solution_ = used;
			best_ = saved;
			++optimize_count_;
		}
		++solve_count_;
		return saved == overtime_;	// stop if it wont be any better
	}

	if (remain.empty()) {
		return false;
	}

	int saveable = 0;
	for (auto i : remain) {
		saveable += ferry_[i].saving;
	}

	if (saveable + saved < overtime_) {
		return false;
	}

	// std::cerr << saveable+saved << " > " << overtime_
	// 	<< " " << solve_count_
	// 	<< " " << optimize_count_
	// 	<< " " << best_
	// 	<< std::endl;

	Indices next_used = used;
	next_used.push_back(-1);	// placeholder

	for (auto i : remain) {
		Indices next_remain;
		std::set_intersection(
			remain.begin(), remain.end(),
			allowed_[i].begin(), allowed_[i].end(),
			std::back_inserter(next_remain));

		next_used.back() = i;
		if (recurse(next_used, next_remain, ferry_[i].saving + saved)) {
			return true;
		}
	}
	return false;
}


void Lake::solve() {
	Indices remain(ferry_.size());
	std::iota(remain.begin(), remain.end(), 0);
	recurse({}, remain, 0);
	std::sort(solution_.begin(), solution_.end());
}


void Lake::statsToStream(std::ostream& out) {
	out << "Need: " << overtime_ << std::endl;
	out << "Best: " << best_ << std::endl;
	out << "Optimized: " << optimize_count_ << std::endl;
	out << "Solved: " << solve_count_ << std::endl;
}


void Lake::solutionToStream(std::ostream& out) {
	out << solution_.size() << '\n';

	for (const auto& ferryIndex: solution_) {
		const auto& ferry = ferry_[ferryIndex];
		const auto& src = ferry.src;
		const auto& dst = ferry.dst;
		const auto& srcName = names_[src];
		const auto& dstName = names_[dst];
		out << srcName << " " << dstName << '\n';
	}
}


int main() {
	Lake lake;
	lake.fromStream(std::cin);
	// lake.ferriesToStream(std::cerr);
	// lake.allowedToStream(std::cerr);

	lake.solve();
	// lake.statsToStream(std::cerr);

	lake.solutionToStream(std::cout);
	return 0;
}
