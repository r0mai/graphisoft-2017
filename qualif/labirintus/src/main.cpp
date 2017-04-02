#include <iostream>
#include <vector>
#include <cassert>
#include <limits>


enum class Field {
    kEdge,
    kFree,
    kWall,
    kStop,
};

enum class Dir {
    kNorth,
    kNorthEast,
    kSouthEast,
    kSouth,
    kSouthWest,
    kNorthWest,
};

struct Pos {
    Pos() = default;
    Pos(int row, int col) : row(row), col(col) {}
    int row = -1;
    int col = -1;
};

std::ostream& operator<<(std::ostream& os, const Pos& p) {
    os << "(" << p.row << ", " << p.col << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Dir& dir) {
    switch (dir) {
        case Dir::kNorth: os << "N"; break;
        case Dir::kNorthEast: os << "NE"; break;
        case Dir::kSouthEast: os << "SE"; break;
        case Dir::kSouth: os << "S"; break;
        case Dir::kSouthWest: os << "SW"; break;
        case Dir::kNorthWest: os << "NW"; break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Field& field) {
    char c = ' ';
    switch (field) {
        case Field::kEdge: c = ' '; break;
        case Field::kFree: c = '.'; break;
        case Field::kWall: c = 'X'; break;
        case Field::kStop: c = 'o'; break;
    }
    return (os << c);
}

Field MakeField(char c) {
    switch (c) {
        case 'W': return Field::kWall;
        case 'C': return Field::kFree;
        case 'M': return Field::kStop;
        default: assert(!"panic");
    }
    return Field::kFree;
}

Dir Opposite(Dir dir) {
    switch (dir) {
        case Dir::kNorth: return Dir::kSouth;
        case Dir::kNorthEast: return Dir::kSouthWest;
        case Dir::kSouthEast: return Dir::kNorthWest;
        case Dir::kSouth: return Dir::kNorth;
        case Dir::kSouthWest: return Dir::kNorthEast;
        case Dir::kNorthWest: return Dir::kSouthEast;
    }
}

Pos Neighbor(const Pos& pos, Dir dir) {
    int even = pos.col % 2 == 0 ? 1 : 0;
    int odd = pos.col % 2 == 0 ? 0 : 1;

    switch (dir) {
        case Dir::kNorth: return Pos(pos.row - 1, pos.col);
        case Dir::kSouth: return Pos(pos.row + 1, pos.col);
        case Dir::kNorthEast: return Pos(pos.row - even, pos.col + 1);
        case Dir::kNorthWest: return Pos(pos.row - even, pos.col - 1);
        case Dir::kSouthEast: return Pos(pos.row + odd, pos.col + 1);
        case Dir::kSouthWest: return Pos(pos.row + odd, pos.col - 1);
    }
}


struct Ray {
    Ray() = default;

    static const int kUnreached = std::numeric_limits<int>::max();
    int bounce = kUnreached;
    uint8_t used = 0;

    bool IsUsed(Dir dir) const {
        uint8_t mask = 1 << int(dir);
        return (used & mask) == mask;
    }

    bool IsUnused(Dir dir) const {
        return !IsUsed(dir);
    }

    void SetUsed(Dir dir) {
        used |= (1 << int(dir));
    }

    void NegateUsed() {
        used = ~used;
    }

    std::vector<Dir> GetUnusedVector() const {
        std::vector<Dir> dirs;
        for (int i = 0; i < 6; ++i) {
            Dir dir = Dir(i);
            if (!IsUsed(dir)) {
                dirs.push_back(dir);
            }
        }
        return dirs;
    }
};


class HexGrid {
public:
    void FromStream(std::istream& in);

    int Cols() const { return cols_; }
    int Rows() const { return rows_; }
    Pos Size() const { return Pos(rows_, cols_); }

    Field GetField(const Pos& pos) const;
    const Ray& GetRay(const Pos& pos) const;
    Ray& GetRay(const Pos& pos);

    void InitRays();
    bool TraceNext();

private:

    std::vector<Pos> GetRowVector(int row) const;
    std::vector<Pos> GetColVector(int col) const;
    std::vector<Pos> GetEdgeVector() const;

    std::vector<std::vector<Field>> grid_;
    std::vector<std::vector<Ray>> ray_grid_;
    std::vector<Pos> edge_;

    int cols_ = 0;
    int rows_ = 0;
};


Field HexGrid::GetField(const Pos& pos) const {
    if (pos.row >= 0 && pos.row < rows_ &&
        pos.col >= 0 && pos.col < cols_)
    {
        return grid_[pos.row][pos.col];
    }
    return Field::kEdge;
}

void HexGrid::FromStream(std::istream& in) {
    int k, n;
    in >> k >> n;

    cols_ = 2 * n;
    rows_ = k;

    grid_.resize(k);
    for (auto& r : grid_) {
        r.resize(2 * n);
    }

    for (int row = 0; row < k; ++row) {
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < n; ++j) {
                char c;
                int col = j * 2 + i;
                in >> c;
                grid_[row][col] = MakeField(c);
            }
        }
    }
    InitRays();
}

Ray& HexGrid::GetRay(const Pos& pos) {
    assert(pos.row >= -1 && pos.col >= -1);
    assert(pos.row <= rows_ && pos.col <= cols_);
    return ray_grid_[pos.row + 1][pos.col + 1];
}

const Ray& HexGrid::GetRay(const Pos& pos) const {
    assert(pos.row >= -1 && pos.col >= -1);
    assert(pos.row <= rows_ && pos.col <= cols_);
    return ray_grid_[pos.row + 1][pos.col + 1];
}

std::vector<Pos> HexGrid::GetRowVector(int row) const {
    std::vector<Pos> result;
    for (int i = 0; i < cols_; ++i) {
        result.emplace_back(row, i);
    }
    return result;
}

std::vector<Pos> HexGrid::GetColVector(int col) const {
    std::vector<Pos> result;
    for (int i = 0; i < rows_; ++i) {
        result.emplace_back(i, col);
    }
    return result;
}

std::vector<Pos> HexGrid::GetEdgeVector() const {
    std::vector<Pos> result;
    for (int i = -1; i < cols_; ++i) {
        result.emplace_back(-1, i);
    }
    for (int i = -1; i < rows_; ++i) {
        result.emplace_back(i, cols_);
    }
    for (int i = cols_; i >= 0; --i) {
        result.emplace_back(rows_, i);
    }
    for (int i = rows_; i >= 0; --i) {
        result.emplace_back(i, -1);
    }
    return result;
}

void HexGrid::InitRays() {
    edge_.clear();
    ray_grid_.clear();
    ray_grid_.resize(rows_ + 2);

    for (auto& row : ray_grid_) {
        row.resize(cols_ + 2);
    }

    for (auto pos : GetRowVector(0)) {
        for (auto dir : {Dir::kNorth, Dir::kNorthWest, Dir::kNorthEast}) {
            if (dir != Dir::kNorth && pos.col % 2 == 1) {
                continue;
            }
            GetRay(Neighbor(pos, dir)).SetUsed(Opposite(dir));
        }
    }

    for (auto pos : GetRowVector(rows_ - 1)) {
        for (auto dir : {Dir::kSouth, Dir::kSouthWest, Dir::kSouthEast}) {
            if (dir != Dir::kSouth && pos.col % 2 == 0) {
                continue;
            }
            GetRay(Neighbor(pos, dir)).SetUsed(Opposite(dir));
        }
    }

    for (auto pos : GetColVector(0)) {
        for (auto dir : {Dir::kNorthWest, Dir::kSouthWest}) {
            GetRay(Neighbor(pos, dir)).SetUsed(Opposite(dir));
        }
    }

    for (auto pos : GetColVector(cols_ - 1)) {
        for (auto dir : {Dir::kNorthEast, Dir::kSouthEast}) {
            GetRay(Neighbor(pos, dir)).SetUsed(Opposite(dir));
        }
    }

    for (auto pos : GetEdgeVector()) {
        auto& ray = GetRay(pos);
        ray.NegateUsed();
        ray.bounce = 0;
        edge_.push_back(pos);
    }
}

bool HexGrid::TraceNext() {
    std::vector<Pos> next;

    for (auto pos : edge_) {
        auto& ray = GetRay(pos);
        int bounce = ray.bounce + 1;
        for (auto dir : ray.GetUnusedVector()) {
            auto step = pos;
            while (GetRay(step).IsUnused(dir)) {
                GetRay(step).SetUsed(dir);
                step = Neighbor(step, dir);

                auto field = GetField(step);
                if (field == Field::kEdge || field == Field::kWall) {
                    break;
                }

                auto& step_ray = GetRay(step);
                step_ray.SetUsed(Opposite(dir));
                if (step_ray.bounce > bounce) {
                    step_ray.bounce = bounce;
                    next.push_back(step);
                }

                if (field == Field::kStop) {
                    break;
                }
            }
        }
    }

    std::swap(edge_, next);
    return !edge_.empty();
}



#if !defined(GUI_ENABLED)

int main() {
    HexGrid map;
    map.FromStream(std::cin);

    for (int row = -1; row < 10; ++row) {
        for (int col = -1; col < 15; ++col) {
            std::cout << map.GetField({row, col});
        }
        std::cout << std::endl;
    }
}

#endif
