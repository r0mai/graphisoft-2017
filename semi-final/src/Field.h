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
std::ostream& operator<<(std::ostream& os, const Field& f) {
	switch (int(f)) {
		case  1: return os << R"(╹)";
		case  2: return os << R"(╸)";
		case  3: return os << R"(┛)";
		case  4: return os << R"(╻)";
		case  5: return os << R"(┃)";
		case  6: return os << R"(┓)";
		case  7: return os << R"(┫)";
		case  8: return os << R"(╺)";
		case  9: return os << R"(┗)";
		case 10: return os << R"(━)";
		case 11: return os << R"(┻)";
		case 12: return os << R"(┏)";
		case 13: return os << R"(┣)";
		case 14: return os << R"(┳)";
		case 15: return os << R"(╋)";
		default: return os << "???";
	}
}
