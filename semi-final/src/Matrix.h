#pragma once

#include "Point.h"
#include <vector>
#include <cassert>
#include <ostream>
#include <algorithm>

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

	template<typename Func>
	void ForeachField(Func func) {
		Point p;
		for (p.y = 0; p.y < height_; ++p.y) {
			int offset = p.y * width_;
			for (p.x = 0; p.x < width_; ++p.x) {
				func(p, fields_[p.x + offset]);
			}
		}
	}

	const std::vector<T>& GetFields() const {
		return fields_;
	}

	T Push(const Point& pos, T value);
	void Rotate(const Point& pos);
	void RotateBack(const Point& pos);

	void Fill(const T& value) {
		std::fill(fields_.begin(), fields_.end(), value);
	}

	bool operator==(const Matrix& other) {
		return
			width_ == other.width_ &&
			height_ == other.height_ &&
			fields_ == other.fields_;
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

template<typename T>
T Matrix<T>::Push(const Point& pos, T value) {
	if (pos.x == -1) {
		assert(pos.y >= 0 && pos.y < height_);
		for (int x = width_; x-- > 1; ) {
			std::swap(At(x, pos.y), At(x - 1, pos.y));
		}
		std::swap(At(0, pos.y), value);
	} else if (pos.x == width_) {
		assert(pos.y >= 0 && pos.y < height_);
		for (int x = 0; x < width_ - 1; ++x) {
			std::swap(At(x, pos.y), At(x + 1, pos.y));
		}
		std::swap(At(width_ - 1, pos.y), value);
	} else if (pos.y == -1) {
		assert(pos.x >= 0 && pos.x < width_);
		for (int y = height_; y-- > 1; ) {
			std::swap(At(pos.x, y), At(pos.x, y - 1));
		}
		std::swap(At(pos.x, 0), value);
	} else if (pos.y == height_) {
		assert(pos.x >= 0 && pos.x < width_);
		for (int y = 0; y < height_ - 1; ++y) {
			std::swap(At(pos.x, y), At(pos.x, y + 1));
		}
		std::swap(At(pos.x, height_ - 1), value);
	}
	return value;
}

template<typename T>
void Matrix<T>::Rotate(const Point& pos) {
	Point opposite = pos;
	if (pos.x == -1) {
		opposite.x = width_-1;
	} else if (pos.x == width_) {
		opposite.x = 0;
	} else if (pos.y == -1) {
		opposite.y = height_-1;
	} else if (pos.y == height_) {
		opposite.y = 0;
	}
	Push(pos, At(opposite));
}

template<typename T>
void Matrix<T>::RotateBack(const Point& pos) {
	Point opposite = pos;
	if (pos.x == -1) {
		opposite.x = width_;
	} else if (pos.x == width_) {
		opposite.x = -1;
	} else if (pos.y == -1) {
		opposite.y = height_;
	} else if (pos.y == height_) {
		opposite.y = -1;
	}
	Rotate(opposite);
}
