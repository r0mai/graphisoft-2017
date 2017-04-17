#pragma once

#include "Point.h"
#include <vector>
#include <cassert>
#include <ostream>

template<typename T>
class Matrix {
public:
	Matrix() = default;
	Matrix(int width, int height, const T& default_value = T{})
		: width_(width)
		, height_(height)
		, fields_(width_ * height_, default_value)
	{}

	T& At(const Point& p) {
		return At(p.x, p.y);
	}

	const T& At(const Point& p) const {
		return At(p.x, p.y);
	}

	T& At(int x, int y) {
        assert(x >= 0 && y >= 0 && x < width_ && y < height_);
        return fields_[x + y * width_];
	}

	const T& At(int x, int y) const {
        assert(x >= 0 && y >= 0 && x < width_ && y < height_);
        return fields_[x + y * width_];
	}

    int Width() const { return width_; }
    int Height() const { return height_; }

	void SetFields(std::vector<T> fields) {
		assert(fields.size() == width_ * height_);
		fields_ = std::move(fields);
	}

	const std::vector<T>& GetFields() const {
		return fields_;
	}

private:
	int width_;
	int height_;
	std::vector<T> fields_;
};

template<typename T>
std::ostream& operator<<(std::ostream& os, const Matrix<T>& m) {
	for (int y = 0; y < m.Height(); ++y) {
		for (int x = 0; x < m.Width(); ++x) {
			os << m.At(x, y) << ' ';
		}
		os << '\n';
	}
	return os;
}
