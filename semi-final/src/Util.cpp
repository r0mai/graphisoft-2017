#include "Util.h"

#include <cassert>

bool IsEastOpen(Field type) {
	return type & 0b1000;
}

bool IsWestOpen(Field type) {
	return type & 0b0010;
}

bool IsNorthOpen(Field type) {
	return type & 0b0001;
}

bool IsSouthOpen(Field type) {
	return type & 0b0100;
}

Field RotateLeft(Field tile) {
    return Field((tile >> 3) + ((tile << 1) & 0xf));
}

Field RotateRight(Field tile) {
    return Field(((tile & 1) << 3) + (tile >> 1));
}

Field Normalize(Field tile) {
	static const int normalized[] = {
		0, 1, 1, 3, 1, 5, 3, 7, 1, 3, 5, 7, 3, 7, 7, 15
	};
	return Field(normalized[tile]);
}

Field NorthFacing(Field tile) {
	while (!IsNorthOpen(tile)) {
		tile = RotateLeft(tile);
	}
	return tile;
}

Field SouthFacing(Field tile) {
	while (!IsSouthOpen(tile)) {
		tile = RotateLeft(tile);
	}
	return tile;
}

Field WestFacing(Field tile) {
	while (!IsWestOpen(tile)) {
		tile = RotateLeft(tile);
	}
	return tile;
}

Field EastFacing(Field tile) {
	while (!IsEastOpen(tile)) {
		tile = RotateLeft(tile);
	}
	return tile;
}

std::vector<Field> GetRotations(Field tile) {
	assert(int(tile) >= 1 && int(tile) <= 15);
	std::vector<Field> rotations;
	rotations.reserve(4);

	Field rotation = tile;
	do {
		rotations.push_back(rotation);
		rotation = RotateLeft(rotation);
	} while (rotation != tile);

	return rotations;
}

std::vector<PushVariation> GetPushVariations(
	const Point& field_size, Field extra)
{
	return GetPushVariations({{0, 0}, field_size}, field_size, extra);
}

std::vector<PushVariation> GetPushVariations(
	const Bounds& bounds, const Point& field_size, Field extra)
{
	int w = field_size.x;
	int h = field_size.y;

	std::vector<PushVariation> variations;
	PushVariation v;

	auto rotations = GetRotations(extra);

	for (int x = bounds.mins.x; x < bounds.maxs.x; ++x) {
		for (Field f : rotations) {
			{
				v.tile = f;
				v.edge = Point{x, -1};
				v.opposite_edge = Point{x, h};
				variations.push_back(v);
			}
			{
				v.tile = f;
				v.edge = Point{x, h};
				v.opposite_edge = Point{x, -1};
				variations.push_back(v);
			}
		}
	}
	for (int y = bounds.mins.y; y < bounds.maxs.y; ++y) {
		for (Field f : rotations) {
			{
				v.tile = f;
				v.edge = Point{-1, y};
				v.opposite_edge = Point{w, y};
				variations.push_back(v);
			}
			{
				v.tile = f;
				v.edge = Point{w, y};
				v.opposite_edge = Point{-1, y};
				variations.push_back(v);
			}
		}
	}

	return variations;
}

std::vector<PushVariation> GetPushVariations(
	int row_bits, int col_bits, const Point& field_size, Field extra)
{
	int w = field_size.x;
	int h = field_size.y;
	std::vector<PushVariation> variations;

	auto rotations = GetRotations(extra);

	for (int x = 0; x < w; ++x) {
		if ((col_bits & (1 << x)) == 0) {
			continue;
		}
		for (Field f : rotations) {
			variations.emplace_back(Point{x, -1}, Point{x, h}, f);
			variations.emplace_back(Point{x, h}, Point{x, -1}, f);
		}
	}
	for (int y = 0; y < h; ++y) {
		if ((row_bits & (1 << y)) == 0) {
			continue;
		}
		for (Field f : rotations) {
			variations.emplace_back(Point{-1, y}, Point{w, y}, f);
			variations.emplace_back(Point{w, y}, Point{-1, y}, f);
		}
	}

	return variations;
}


std::ostream& operator<<(std::ostream& os, const PushVariation& push) {
	os << "(" << push.edge << ", " << push.tile << ")";
	return os;
}
