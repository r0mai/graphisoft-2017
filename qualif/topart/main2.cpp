#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <numeric>
#include <tuple>
#include <chrono>
#include <boost/dynamic_bitset.hpp>


struct Ferry {
	Ferry() = default;
	Ferry(int src, int dst, int saving, int duration, int replacing)
		: src(src), dst(dst), saving(saving), duration(duration)
		, replacing(replacing)
	{}

	int src = 0;
	int dst = 0;
	int saving = 0;
	int duration = 0;
	int replacing = 0;
};


std::ostream& operator<<(std::ostream& stream, const Ferry& ff) {
	stream << "#" << ff.src << " #" << ff.dst << " " << ff.duration;
	return stream;
}

using Indices = boost::dynamic_bitset<>;

using Clock = std::chrono::steady_clock;
using Duration = std::chrono::duration<double>;
using RouteInfo = std::pair<int, int>; // duration, ferry_index
using RouteMap = std::map<int, std::map<int, RouteInfo>>;

class Lake {
public:
	Lake();
	void fromStream(std::istream& in);
	void ferriesToStream(std::ostream& out);
	void allowedToStream(std::ostream& out);
	void statsToStream(std::ostream& out);
	void solve();
	void solutionToStream(std::ostream& out);

private:
	RouteMap createRouteMap();
	void filterFerries();
	void calculateWorst();
	void calculateAllowed();
	bool recurse(const Indices& used, const Indices& remain, int saved, int replaced);

	std::vector<std::string> names_;
	std::vector<int> road_;
	std::vector<Ferry> ferry_;
	std::vector<Indices> allowed_;

	Indices solution_;
	int best_ = 0;
	int optimize_count_ = 0;
	int solve_count_ = 0;
	int recurse_count_ = 0;
	int last_optimized_ = 0;

	int time_ = 0;
	int overtime_ = 0;
	int road_sum_ = 0;
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
			ferry_.emplace_back(p, q, saving, d, sums[q] - sums[p]);
		}
	}


	auto fitness = [](const Ferry& f) {
		return std::make_tuple(
			-f.dst + f.src, -f.saving, f.src, f.dst);
	};

	std::sort(ferry_.begin(), ferry_.end(),
		[&](const Ferry& lhs, const Ferry& rhs) {
			return fitness(lhs) < fitness(rhs);
		});

	in >> time_;
	overtime_ = std::max(sums[n] - time_, 0);
	road_sum_ = sums[n];

	calculateAllowed();
}


void addIndex(Indices& indices, int i) {
	indices.set(i);
}


void Lake::calculateAllowed() {
	auto fs = ferry_.size();
	allowed_.resize(fs);

	for (std::size_t i = 0; i < fs; ++i) {
		auto& indices = allowed_[i];
		int src = ferry_[i].src;
		int dst = ferry_[i].dst;

		indices.resize(fs);

		for (std::size_t j = 0; j < fs; ++j) {
			const auto& ferry = ferry_[j];
			if (i != j && (ferry.dst <= src || ferry.src >= dst)) {
				addIndex(indices, j);
			}
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
		for (int j = 0, je = allowedFerries.size(); j < je; ++j) {
			if (allowedFerries.test(j)) {
				const auto& allowedFerry = ferry_[j];
				out << "    " << allowedFerry.src << "->"
						<< allowedFerry.dst << "\n";
			}
		}
		out << "}\n";
	}
}


bool Lake::recurse(const Indices& used, const Indices& remain, int saved, int replaced) {
	++recurse_count_;
	if (--loop_ < 0) {
		loop_ = 10000;
		auto current_time = Clock::now();
		auto delta = std::chrono::duration_cast<Duration>(current_time - start_time_);

		if (delta > Duration(9.85)) {
			return true;
		}
	}

	int current = road_sum_ - replaced;
	if (current <= best_) {
		return false;
	}

	if (saved >= overtime_) {
		solution_ = used;
		best_ = current;
		// std::cerr << road_sum_ << " " << saved << " " << current << std::endl;
		++optimize_count_;
		return best_ == road_sum_;	// stop if it wont be any better
	}

	if (remain.empty()) {
		return false;
	}

#if 1
	long int saveable = 0;
	for (size_t i = 0, ie = remain.size(); i < ie; ++i) {
		if (remain.test(i)) {
			saveable += ferry_[i].saving;
		}
	}

	if (saveable + saved < overtime_) {
		return false;
	}
#endif

	Indices next_used = used;
	for (size_t i = 0, ie = remain.size(); i < ie; ++i) {
		if (!remain.test(i)) {
			continue;
		}

		Indices next_remain = remain;
		next_remain &= allowed_[i];
		next_used.set(i);

		if (recurse(next_used, next_remain,
				ferry_[i].saving + saved,
				ferry_[i].replacing + replaced)) {
			return true;
		}

		next_used.reset(i);
	}

	return false;
}


RouteMap Lake::createRouteMap() {
	RouteMap route;

	int i = 0;
	for (auto d : road_) {
		route[i][i + 1] = {d, -1};
		++i;
	}

	i = 0;
	for (const auto& f : ferry_) {
		route[f.src][f.dst] = {f.duration, i};
		++i;
	}

	return route;
}


void Lake::filterFerries() {
#if 0
	// Dijkstra
	using PrevInfo = std::pair<int, int>; // prev_city_index, ferry_index
	using WeightInfo = std::pair<int, int>; // weight, city_index
	auto route = createRouteMap();

	auto fs = ferry_.size();

	int used = 0;
	int unused = 0;

	for (size_t i = 0; i < fs; ++i) {
		auto src = ferry_[i].src;
		auto dst = ferry_[i].dst;

		int vertices = dst - src + 1;
		std::vector<int> min_distance(vertices, std::numeric_limits<int>::max());
		std::vector<PrevInfo> previous(vertices, {-1, -1});
		std::set<WeightInfo> vertex_queue;

		int p = 0;
		min_distance[p] = 0;
		vertex_queue.insert({min_distance[p], p});

		while (!vertex_queue.empty()) {
			int dist_u, u;
			std::tie(dist_u, u) = *vertex_queue.begin();
			vertex_queue.erase(vertex_queue.begin());

			// Visit each edge exiting u
			for (const auto& info : route[u + src]) {
				int v = info.first;
				if (v > dst) {
					continue;
				}
				v -= src;
				int dist_v = info.second.first + dist_u;
				int ferry_index = info.second.second;

				if (dist_v < min_distance[v]) {
					vertex_queue.erase({min_distance[v], v});
					min_distance[v] = dist_v;
					previous[v] = {u, ferry_index};
					vertex_queue.insert({min_distance[v], v});
				}
			}
		}

		if (previous[vertices - 1].second == i) {
			++used;
		} else {
			++unused;
		}
	}
	std::cerr << used << std::endl;
	std::cerr << unused << std::endl;
#endif
}


void Lake::calculateWorst() {
	// Dijkstra
	using PrevInfo = std::pair<int, int>; // prev_city_index, ferry_index
	using WeightInfo = std::pair<int, int>; // weight, city_index

	auto route = createRouteMap();

	int vertices = road_.size() + 1;
	std::vector<int> min_distance(vertices, std::numeric_limits<int>::max());
	std::vector<PrevInfo> previous(vertices, {-1, -1});
	std::set<WeightInfo> vertex_queue;

	int p = 0;
	min_distance[p] = 0;
	vertex_queue.insert({min_distance[p], p});

	while (!vertex_queue.empty()) {
		int dist_u, u;
		std::tie(dist_u, u) = *vertex_queue.begin();
		vertex_queue.erase(vertex_queue.begin());

		// Visit each edge exiting u
		for (const auto& info : route[u]) {
			int v = info.first;
			int dist_v = info.second.first + dist_u;
			int ferry_index = info.second.second;

			if (dist_v < min_distance[v]) {
				vertex_queue.erase({min_distance[v], v});
				min_distance[v] = dist_v;
				previous[v] = {u, ferry_index};
				vertex_queue.insert({min_distance[v], v});
			}
		}
	}

	p = vertices - 1;
	std::set<int> fs;
	while (p > 0) {
		fs.insert(previous[p].second);
		p = previous[p].first;
	}
	fs.erase(-1);

	int replaced = 0;
	int duration = 0;

	solution_.clear();
	solution_.resize(ferry_.size());

	for (auto x : fs) {
		addIndex(solution_, x);
		replaced += ferry_[x].replacing;
		duration += ferry_[x].duration;
	}
	best_ = road_sum_ - replaced;
	// std::cerr << "Dijkstra: " << best_ << " " << duration << std::endl;
}


void Lake::solve() {
	calculateWorst();

	Indices used;
	Indices remain(ferry_.size());

	used.resize(ferry_.size());
	remain.set();

	recurse(used, remain, 0, 0);
}


void Lake::statsToStream(std::ostream& out) {
	int ferry_time = 0;
	for (size_t i = 0; i < solution_.size(); ++i) {
		if (solution_.test(i)) {
			ferry_time += ferry_[i].duration;
		}
	}

	int total = best_ + ferry_time;

	out << "Limit: " << time_ << std::endl;
	out << "Total: " << total << std::endl;
	out << "Biked: " << best_ << std::endl;
	out << "Ferry: " << ferry_time << std::endl;
	out << "Optimized: " << optimize_count_ << std::endl;
	out << "Recurse:   " << recurse_count_ << std::endl;
}


void Lake::solutionToStream(std::ostream& out) {
	std::set<std::pair<int, int>> result;

	for (size_t i = 0, ie = solution_.size(); i < ie; ++i) {
		if (solution_.test(i)) {
			result.insert(std::make_pair(ferry_[i].src, ferry_[i].dst));
		}
	}

	out << result.size() << "\n";
	for (const auto& x : result) {
		const auto& src = names_[x.first];
		const auto& dst = names_[x.second];
		out << src << " " << dst << "\n";
	}
}


int main() {
	Lake lake;
	lake.fromStream(std::cin);
	// lake.ferriesToStream(std::cerr);
	// lake.allowedToStream(std::cerr);

	lake.solve();
	lake.statsToStream(std::cerr);

	lake.solutionToStream(std::cout);
	return 0;
}
