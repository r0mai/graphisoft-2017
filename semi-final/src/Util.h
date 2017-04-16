#pragma once

int RotateLeft(int tile) {
    return (tile >> 3) + ((tile << 1) & 0xf);
}

int RotateRight(int tile) {
    return ((tile & 1) << 3) + (tile >> 1);
}
