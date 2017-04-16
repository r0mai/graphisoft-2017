#include "Util.h"

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
