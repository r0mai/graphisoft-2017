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

std::vector<Field> GetRotations(Field tile);

struct PushVariation {
	Point edge;
	Point opposite_edge;
	Field tile;
};

std::vector<PushVariation> GetPushVariations(
	const Bounds& bounds, const Point& field_size, Field extra);
std::vector<PushVariation> GetPushVariations(const Point& field_size, Field extra);
