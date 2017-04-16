#include "Util.h"

bool NoPositiveXBorder(int type) {
	return type & 0b1000;
}

bool NoNegativeXBorder(int type) {
	return type & 0b0010;
}

bool NoPositiveYBorder(int type) {
	return type & 0b0001;
}

bool NoNegativeYBorder(int type) {
	return type & 0b0100;
}

int RotateLeft(int tile) {
    return (tile >> 3) + ((tile << 1) & 0xf);
}

int RotateRight(int tile) {
    return ((tile & 1) << 3) + (tile >> 1);
}
