#pragma once

#include "Field.h"

#include <vector>

bool IsNorthOpen(Field type);
bool IsSouthOpen(Field type);
bool IsWestOpen(Field type);
bool IsEastOpen(Field type);

Field RotateLeft(Field tile);
Field RotateRight(Field tile);

std::vector<Field> GetRotations(Field tile);
