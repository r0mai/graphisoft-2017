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

	std::shared_ptr<Village> current = villages.front();
	int timeSpent = 0;
	while (current != villages.back()) {
		timeSpent += current->getNextDistance();
		current = current->getNext();
	}
	std::cerr << "Going through all villages would take:"
			<< timeSpent << std::endl;
}
