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


std::shared_ptr<Village> findByName(
		const std::vector<std::shared_ptr<Village>>& villages,
		const std::string& name) {
	for (const auto& village: villages) {
		if (village->getName() == name) {
			return village;
		}
	}
	return nullptr;
}

using JumpDescriptor =
		std::pair<std::shared_ptr<Village>, std::shared_ptr<Village>>;

std::pair<
		std::vector<JumpDescriptor>,
		int> getJumps(std::shared_ptr<Village> start, int budget) {
	if (start == nullptr) {
		// No more villages to visit.
		return std::make_pair(std::vector<JumpDescriptor>{}, 0);
	}
	const auto& jumpsAvailable = start->getJumps();

	const auto& straightRoute = getJumps(start->getNext(),
			budget - start->getNextDistance());
	const auto& straightJumps = straightRoute.first;
	const auto& straightCost = straightRoute.second + start->getNextDistance();

	// Greedy, if straight is acceptable, or we have no other choice,
	// we choose it.
	if (straightCost <= budget || jumpsAvailable.empty()) {
		return std::make_pair(straightJumps, straightCost);
	}

	std::vector<std::pair<std::vector<JumpDescriptor>, int>> indirectRoutes;
	for (const auto& jump: jumpsAvailable) {
		const auto& destination = jump.first;
		const auto& jumpCost = jump.second;
		const auto& route = getJumps(destination, budget - jumpCost);
		auto routeJumps = route.first;
		routeJumps.insert(routeJumps.begin(),
				std::make_pair(start, destination));
		const auto& routeCost = route.second;
		indirectRoutes.push_back(
				std::make_pair(routeJumps, routeCost + jumpCost));
	}

	std::sort(indirectRoutes.begin(), indirectRoutes.end(),
			[](const std::pair<std::vector<JumpDescriptor>, int>& l,
				const std::pair<std::vector<JumpDescriptor>, int>& r) {
				return l.second < r.second;
			});

	const auto& route = indirectRoutes[0];
	return route;

}


int main() {
	std::vector<std::shared_ptr<Village>> villages;
	int n;
	std::cin >> n;

	for (int i=0; i<n; ++i) {
		std::string cityName;
		std::cin >> cityName;
		villages.push_back(std::make_shared<Village>(cityName));
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
		auto fromVillage = findByName(villages, from);
		auto toVillage = findByName(villages, to);
		assert(fromVillage && toVillage && "One of the villages was not found");

		fromVillage->addJump(toVillage, distance);
	}

	int timeBudget;
	std::cin >> timeBudget;

	const auto& route = getJumps(villages[0], timeBudget);
	std::cerr << "Total route cost is: " << route.second << std::endl;
	std::cout << route.first.size() << std::endl;
	for (const auto& jump: route.first) {
		const auto& from = jump.first->getName();
		const auto& to = jump.second->getName();
		std::cout << from << " " << to << std::endl;
	}
}
