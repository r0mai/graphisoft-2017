#include <iostream>
#include <vector>
#include <cassert>

enum class Field {
    kEdge,
    kFree,
    kWall,
    kStop
};

enum class Dir {
    kNorth,
    kNorthEast,
    kSouthEast,
    kSouth,
    kSouthWest,
    kNorthWest
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


Pos Neighbor(const Pos& pos, Dir dir) {
    int even = pos.col % 2 ? 1 : 0;
    int odd = pos.col % 2 ? 0 : 1;

    switch (dir) {
        case Dir::kNorth: return Pos(pos.row - 1, pos.col);
        case Dir::kSouth: return Pos(pos.row + 1, pos.col);
        case Dir::kNorthEast: return Pos(pos.row - even, pos.col + 1);
        case Dir::kNorthWest: return Pos(pos.row - even, pos.col - 1);
        case Dir::kSouthEast: return Pos(pos.row + odd, pos.col + 1);
        case Dir::kSouthWest: return Pos(pos.row + odd, pos.col - 1);
    }
}


class HexGrid {
public:
    void FromStream(std::istream& in);

    int Cols() const { return cols_; }
    int Rows() const { return rows_; }
    Pos Size() const { return Pos(rows_, cols_); }

    Field GetField(const Pos& pos) {
        if (pos.row >= 0 && pos.row < rows_ &&
            pos.col >= 0 && pos.col < cols_)
        {
            return grid_[pos.row][pos.col];
        }
        return Field::kEdge;
    }

private:
    std::vector<std::vector<Field>> grid_;
    int cols_ = 0;
    int rows_ = 0;
};


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
