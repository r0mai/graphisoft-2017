#include "Grid.h"



int Grid::Width() const {
    return fields_.Width();
}

int Grid::Height() const {
    return fields_.Height();
}

Point Grid::Size() const {
    return {fields_.Width(), fields_.Height()};
}

void Grid::Init(int width, int height, int displays, int players) {
    fields_ = Matrix<int>(width, height, 0);
    display_.resize(displays, {-1, -1});
    position_.resize(players, {-1, -1});
}

void Grid::Randomize() {
    for (int x = 0; x < fields_.Width(); ++x) {
        for (int y = 0; y < fields_.Height(); ++y) {
            fields_.At(x, y) = 1 + (rand() % 15);
        }
    }
}

void Grid::UpdateFields(std::vector<int> fields) {
    fields_.SetFields(std::move(fields));
}

void Grid::UpdateDisplay(int index, const Point& pos) {
    display_[index] = pos;
}

void Grid::UpdatePosition(int player, const Point& pos) {
    position_[player] = pos;
}

int Grid::At(int x, int y) const {
    return fields_.At(x, y);
}

int Grid::Push(const Point& pos, int t) {
    // if (pos.x == -1) {}
    return 0;
}
