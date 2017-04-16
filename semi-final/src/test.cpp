#include "FloodFill.h"
#include <cstdlib>
#include <iostream>
#include <chrono>

int TestFloodFillTime() {
	int sum = 0;
	auto start = std::chrono::steady_clock::now();
	for (int i = 0; i < 1000; ++i) {
		Matrix<int> field(15, 15, 15); // worst case map for flood fill
		auto result = FloodFill(field, {rand() % 15, rand() % 15});
		sum += result.At({rand() % 15, rand() % 15}) << '\n';
	}
	auto end = std::chrono::steady_clock::now();

	std::cout << "FloodFill took " <<
		std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() <<
		"ms" << std::endl;

	return sum;
}

int main() {
	TestFloodFillTime();
}
