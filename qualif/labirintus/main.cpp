#include <iostream>
#include <vector>
#include <cassert>

enum class Field {
    kWall, kFree, kScreen
};

struct Point {
    Point() = default;
    Point(int x, int y) : x(x), y(y) {}
    int x = -1;
    int y = -1;
};

std::ostream& operator<<(std::ostream& os, const Point& p) {
    os << "(" << p.x << ", " << p.y << ")";
    return os;
}

class HexaMatrix {
public:
    HexaMatrix(std::istream& in);

    bool IsValid(const Point& p) const;
    Field GetFieldAt(const Point& p) const;

    std::vector<Point> GetAdjacents(const Point& p) const;
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

bool HexaMatrix::IsValid(const Point& p) const {
    return
        p.x >= 0 && p.y >= 0 &&
        p.y < int(grid.size()) && p.x < int(grid[0].size());
}

std::vector<Point> HexaMatrix::GetAdjacents(const Point& p) const {
    int w = grid[0].size();

    std::vector<Point> result;
    result.reserve(6);

    result.push_back({p.x, p.y - 1});
    result.push_back({p.x, p.y + 1});

    if (p.x < w/2) {
        int x_before = p.x + w/2 - 1;
        int x_after = p.x + w/2;

        result.push_back({x_before, p.y});
        result.push_back({x_before, p.y - 1});

        result.push_back({x_after, p.y});
        result.push_back({x_after, p.y - 1});

    } else {
        int x_before = p.x - w/2;
        int x_after = p.x - w/2 + 1;

        result.push_back({x_before, p.y});
        result.push_back({x_before, p.y + 1});

        result.push_back({x_after, p.y});
        result.push_back({x_after, p.y + 1});
    }

    return result;
}

Field HexaMatrix::GetFieldAt(const Point& p) const {
    assert(IsValid(p));
    return grid[p.y][p.x];
}

int main() {
    HexaMatrix map(std::cin);

    auto adj = map.GetAdjacents(Point{2, 2});

    for (auto p : adj) {
        std::cout << p << std::endl;
    }
}
