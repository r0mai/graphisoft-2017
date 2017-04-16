#pragma once

#include "Field.h"

bool NoPositiveXBorder(Field type);
bool NoNegativeXBorder(Field type);
bool NoPositiveYBorder(Field type);
bool NoNegativeYBorder(Field type);

Field RotateLeft(Field tile);
Field RotateRight(Field tile);
