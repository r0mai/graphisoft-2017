#pragma once

#include <ostream>
#include <istream>

enum Field {};

inline
std::istream& operator>>(std::istream& is, Field& f) {
	int x;
	if (is >> x) {
		f = Field(x);
	}
	return is;
}

inline
std::ostream& operator<<(std::ostream& os, Field& f) {
	return os << int(f);
}
