#pragma once

#include "Point.h"
#include "Field.h"
#include "Bounds.h"

#include <vector>

bool IsNorthOpen(Field type);
bool IsSouthOpen(Field type);
bool IsWestOpen(Field type);
bool IsEastOpen(Field type);

Field RotateLeft(Field tile);
Field RotateRight(Field tile);
Field Normalize(Field tile);

Field NorthFacing(Field tile);
Field SouthFacing(Field tile);
Field WestFacing(Field tile);
Field EastFacing(Field tile);

std::vector<Field> GetRotations(Field tile);

int OpenCount(Field tile);

struct PushVariation {
	PushVariation() = default;
	PushVariation(Point edge, Point opposite, Field tile)
		: edge(edge)
		, opposite_edge(opposite)
		, tile(tile)
	{}

	Point edge;
	Point opposite_edge;
	Field tile;
};

std::vector<PushVariation> GetPushVariations(
	const Bounds& bounds, const Point& field_size, Field extra);
std::vector<PushVariation> GetPushVariations(
	int row_bits, int col_bits, const Point& field_size, Field extra);
std::vector<PushVariation> GetPushVariations(const Point& field_size, Field extra);

std::ostream& operator<<(std::ostream& os, const PushVariation& push);

template<typename F>
void ForEachPoint(const Point& size, F fn) {
	for (int y = 0; y < size.y; ++y) {
		for (int x = 0; x < size.x; ++x) {
			fn(Point{x, y});
		}
	}
}
