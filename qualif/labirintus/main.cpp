#include <iostream>
#include <vector>
#include <cassert>

enum class Field {
    kWall, kFree, kScreen
};

class HexaMatrix {
public:
    HexaMatrix(std::istream& in);
private:
    std::vector<std::vector<Field>> grid;
};

HexaMatrix::HexaMatrix(std::istream& in) {
    int k, n;
    in >> k >> n;

    grid.resize(k);
    for (auto& r : grid) {
        r.resize(2*n);
    }

    for (int y = 0; y < k; ++y) {
        for (int x = 0; x < 2*n; ++x) {
            char c;
            in >> c;
            switch (c) {
                case 'W': grid[y][x] = Field::kWall; break;
                case 'C': grid[y][x] = Field::kFree; break;
                case 'M': grid[y][x] = Field::kScreen; break;
                default: assert(!"panic");
            }
        }
    }
}

int main() {
    HexaMatrix map(std::cin);
}
