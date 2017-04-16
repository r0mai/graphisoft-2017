#include "Util.h"

bool NoPositiveXBorder(Field type) {
	return type & 0b1000;
}

bool NoNegativeXBorder(Field type) {
	return type & 0b0010;
}

bool NoPositiveYBorder(Field type) {
	return type & 0b0001;
}

bool NoNegativeYBorder(Field type) {
	return type & 0b0100;
}

Field RotateLeft(Field tile) {
    return Field((tile >> 3) + ((tile << 1) & 0xf));
}

Field RotateRight(Field tile) {
    return Field(((tile & 1) << 3) + (tile >> 1));
}
