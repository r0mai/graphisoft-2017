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

std::ostream& operator<<(std::ostream& os, const PushVariation& push) {
	os << "(" << push.edge << ", " << push.tile << ")";
	return os;
}
