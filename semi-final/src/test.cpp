#include "FloodFill.h"
#include "Field.h"
#include <cstdlib>
#include <iostream>
#include <chrono>

Matrix<Field> WorstCaseMap(int w, int h) {
	Matrix<Field> field(15, 15, Field(10));
	for (int y = 0; y < h; ++y) {
		field.At(0,   y) = (y % 2 == 0) ? Field(12) : Field(9);
		field.At(w-1, y) = (y % 2 == 0) ? Field(3) : Field(6);
	}
	return field;
}

int TestFloodFillTime() {
	int sum = 0;
	auto start = std::chrono::steady_clock::now();
	for (int i = 0; i < 1000; ++i) {
		auto field = WorstCaseMap(15, 15);
		auto result = FloodFill(field, {rand() % 15, rand() % 15});
		sum += result.At({rand() % 15, rand() % 15});
	}
	auto end = std::chrono::steady_clock::now();

	std::cout << "FloodFill took " <<
		std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() <<
		"ms" << std::endl;

	return sum;
}

int main() {
	std::cout << TestFloodFillTime() << std::endl;
}
