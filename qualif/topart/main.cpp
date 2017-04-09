#include <iostream>
#include <algorithm>
#include <numeric>
#include <vector>
#include <memory>
#include <string>
#include <map>

#include <cassert>


class Village {
	std::map<std::shared_ptr<Village>, int> villages;
	std::pair<std::shared_ptr<Village>, int> neighbour = {nullptr, 0};
	std::string name;


public:
	explicit Village(std::string name) : name(std::move(name)) { }

	void setNeighbour(std::shared_ptr<Village> neighbour, int distance) {
		this->neighbour = std::make_pair(neighbour, distance);
	}

	void addJump(std::shared_ptr<Village> village, int distance) {
		villages[village] = distance;
	}

	const std::string& getName() const { return name; }

	std::shared_ptr<Village> getNext() const {
		return neighbour.first;
	}

	int getNextDistance() const { return neighbour.second; }

	std::map<std::shared_ptr<Village>, int> getJumps() const {
		return villages;
	}
};

template class std::vector<std::shared_ptr<Village>>;

std::shared_ptr<Village> findByName(
		const std::map<std::string, std::shared_ptr<Village>>& villagesByName,
		const std::string& name) {
	auto it = villagesByName.find(name);
	if (it == villagesByName.end()) {
		return nullptr;
	}
	return it->second;
}
using JumpDescriptor =
		std::pair<std::shared_ptr<Village>, std::shared_ptr<Village>>;

std::tuple<
		std::vector<JumpDescriptor>,
		int, int> getJumps(std::shared_ptr<Village> start, int budget) {
	if (start == nullptr) {
		// No more villages to visit.
		return std::make_tuple(std::vector<JumpDescriptor>{}, 0, 0);
	}
	static std::map<std::shared_ptr<Village>,
			std::tuple<std::vector<JumpDescriptor>, int, int>> memo;
	auto memoIt = memo.find(start);
	if (memoIt != memo.end()) {
		return memoIt->second;
	}

	const auto& jumpsAvailable = start->getJumps();

	const auto& straightRoute = getJumps(start->getNext(),
			budget - start->getNextDistance());
	const auto& straightJumps = std::get<0>(straightRoute);
	const auto& straightCost = std::get<1>(straightRoute)+ start->getNextDistance();
	const auto& straightCycleTime = std::get<2>(straightRoute) + start->getNextDistance();

	// Greedy, if straight is acceptable, or we have no other choice,
	// we choose it.
	if (straightCost <= budget || jumpsAvailable.empty()) {
		const auto result = std::make_tuple(straightJumps, straightCost, straightCycleTime);
		memo[start] = result;
		return result;
	}

	std::vector<std::tuple<std::vector<JumpDescriptor>, int, int>> indirectRoutes;
	for (const auto& jump: jumpsAvailable) {
		const auto& destination = jump.first;
		const auto& jumpCost = jump.second;
		const auto& route = getJumps(destination, budget - jumpCost);
		auto routeJumps = std::get<0>(route);
		routeJumps.insert(routeJumps.begin(),
				std::make_pair(start, destination));
		const auto& routeCost = std::get<1>(route);
		const auto& routeCycleTime = std::get<2>(route);
		indirectRoutes.push_back(
				std::make_tuple(routeJumps, routeCost + jumpCost, routeCycleTime));
	}

	std::vector<std::tuple<std::vector<JumpDescriptor>, int, int>> permissibleRoutes;
	std::copy_if(indirectRoutes.rbegin(), indirectRoutes.rend(),
			std::back_inserter(permissibleRoutes),
			[&budget](const std::tuple<std::vector<JumpDescriptor>, int, int>& route) {
					return std::get<1>(route) <= budget; });


	if (permissibleRoutes.empty()) {
		const auto result = indirectRoutes[0];
		memo[start] = result;
		return result;
	}

	const auto& it =
			std::max_element(permissibleRoutes.begin(), permissibleRoutes.end(),
			[](const std::tuple<std::vector<JumpDescriptor>, int, int>& l,
				const std::tuple<std::vector<JumpDescriptor>, int, int>& r) {
					return std::get<2>(l) < std::get<2>(r); });

	memo[start] = *it;
	return *it;
}


int main() {
	std::vector<std::shared_ptr<Village>> villages;
	std::map<std::string, std::shared_ptr<Village>> villagesByName;
	int n;
	std::cin >> n;

	for (int i=0; i<n; ++i) {
		std::string cityName;
		std::cin >> cityName;
		auto village = std::make_shared<Village>(cityName);
		villages.push_back(village);
		villagesByName[cityName] = village;
	}

	// Instead of creating a cycle, let's add a deep copy of the first village
	// to the end.
	villages.push_back(std::make_shared<Village>(*villages.front()));

	for (int i=0; i<n; ++i) {
		int distance;
		std::cin >> distance;
		villages[i]->setNeighbour(villages[i+1], distance);
	}

	int jumps;
	std::cin >> jumps;
	for (int i=0; i<jumps; ++i) {
		std::string from;
		std::string to;
		int distance;
		std::cin >> from >> to >> distance;
		auto fromVillage = findByName(villagesByName, from);
		auto toVillage = findByName(villagesByName, to);
		if (toVillage == nullptr || fromVillage == nullptr) {
			std::cerr << "Could not find cities for jump between: "
					<< from << " " << to << std::endl;
			continue;
		}
		if (toVillage == villages.front()) {
			// Jumps only go forward
			toVillage = villages.back();
		}
		if (std::find(villages.begin(), villages.end(), fromVillage) >=
				std::find(villages.begin(), villages.end(), toVillage)) {
			continue;
		}

		fromVillage->addJump(toVillage, distance);
	}

	int timeBudget;
	std::cin >> timeBudget;

	const auto& route = getJumps(villages[0], timeBudget);
	std::cerr << "Total route cost is: " << std::get<1>(route) << std::endl;
	std::cerr << "Total time on bike is: " << std::get<2>(route) << std::endl;
	std::cout << std::get<0>(route).size() << std::endl;
	for (const auto& jump: std::get<0>(route)) {
		const auto& from = jump.first->getName();
		const auto& to = jump.second->getName();
		std::cout << from << " " << to << std::endl;
	}
}
