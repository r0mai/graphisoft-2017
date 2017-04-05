#include <iostream>
#include <vector>
#include <array>
#include <tuple>
#include <cassert>
#include <numeric>
#include <algorithm>

int intCos(int turns) {
    switch (turns) {
        case 0: return 1;
        case 1: return 0;
        case 2: return -1;
        case 3: return 0;
    }
    assert(false);
    return 0;
}

int intSin(int turns) {
    switch (turns) {
        case 0: return 0;
        case 1: return 1;
        case 2: return 0;
        case 3: return -1;
    }
    assert(false);
    return 0;
}


struct Point {
    Point() = default;
    Point(int x, int y, int z) : x(x), y(y), z(z) {}

    int x;
    int y;
    int z;

    int index = -1;

    Point negate() const {
        auto p = *this;
        p.x *= -1;
        p.y *= -1;
        p.z *= -1;
        return p;
    }
};

bool operator<(const Point& lhs, const Point& rhs) {
    return
        std::tie(lhs.x, lhs.y, lhs.z) <
        std::tie(rhs.x, rhs.y, rhs.z);
}

bool operator==(const Point& lhs, const Point& rhs) {
    return
        std::tie(lhs.x, lhs.y, lhs.z) ==
        std::tie(rhs.x, rhs.y, rhs.z);
}

bool operator!=(const Point& lhs, const Point& rhs) {
    return !(lhs == rhs);
}

struct Matrix {
    int a, b, c;
    int d, e, f;
    int g, h, i;
};

std::ostream& operator<<(std::ostream& os, const Point& p) {
    os << "(" << p.x << "," << p.y << "," << p.z << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Matrix& m) {
    os << "((" << m.a << "," << m.b << "," << m.c << "),";
    os << "(" << m.d << "," << m.e << "," << m.f << "),";
    os << "(" << m.g << "," << m.h << "," << m.i << "))";
    return os;
}

int doStuff(const Point& row, const Point& column) {
    int result = 0;
    result += row.x * column.x;
    result += row.y * column.y;
    result += row.z * column.z;
    return result;
}

Matrix multiply(const Matrix& p, const Matrix& v) {
    Point r1{p.a, p.b, p.c};
    Point r2{p.d, p.e, p.f};
    Point r3{p.g, p.h, p.i};

    Point c1{v.a, v.d, v.g};
    Point c2{v.b, v.e, v.h};
    Point c3{v.c, v.f, v.i};

    Matrix r;

    r.a = doStuff(r1, c1);
    r.b = doStuff(r1, c2);
    r.c = doStuff(r1, c3);

    r.d = doStuff(r2, c1);
    r.e = doStuff(r2, c2);
    r.f = doStuff(r2, c3);

    r.g = doStuff(r3, c1);
    r.h = doStuff(r3, c2);
    r.i = doStuff(r3, c3);

    return r;
}

// I know it's the other way around
Point transform(const Point& p, const Matrix& m) {
    Point r;
    r.x = m.a*p.x + m.b*p.y + m.c*p.z;
    r.y = m.d*p.x + m.e*p.y + m.f*p.z;
    r.z = m.g*p.x + m.h*p.y + m.i*p.z;
    return r;
}

Point translate(const Point& p, const Point& delta) {
    Point r = p;
    r.x += delta.x;
    r.y += delta.y;
    r.z += delta.z;
    return r;
}

Matrix rotationMatrixFor(int rx, int ry, int rz) {
    Matrix mx {
                  1,          0,           0,
                  0,  intCos(rx), -intSin(rx),
                  0,  intSin(rx),  intCos(rx)
    };
    Matrix my {
         intCos(ry),          0,  intSin(ry),
                  0,          1,           0,
        -intSin(ry),          0,  intCos(ry)
    };
    Matrix mz {
         intCos(rz), -intSin(rz),         0,
         intSin(rz),  intCos(rz),         0,
                  0,          0,          1,
    };

    return multiply(mz, multiply(my, mx));
}

struct Edge {
    Edge() = default;
    Edge(int start_index, int end_index) :
        start_index(start_index),
        end_index(end_index)
    {}

    int start_index;
    int end_index;
};

struct Hole {
    std::vector<int> edge_indicies;
};

struct Face {
    std::vector<int> edge_indicies;
    std::vector<Hole> holes;
};

struct Building {
    std::vector<Point> vertices;
    std::vector<Edge> edges;
    std::vector<Face> faces;
};

Building read(std::istream& in) {
    Building building;

    int vertex_count;
    in >> vertex_count;

    building.vertices.resize(vertex_count);
    for (auto& p : building.vertices) {
        in >> p.x >> p.y >> p.z;
    }

    int edge_count;
    in >> edge_count;

    building.edges.resize(edge_count);
    for (auto& e : building.edges) {
        in >> e.start_index >> e.end_index;
    }

    int face_count;
    in >> face_count;
    building.faces.resize(face_count);

    for (auto& face : building.faces) {
        int face_edge_count;
        in >> face_edge_count;
        face.edge_indicies.resize(face_edge_count);

        for (auto& edge_index : face.edge_indicies) {
            in >> edge_index;
        }

        int hole_count;
        in >> hole_count;
        face.holes.resize(hole_count);

        for (auto& hole : face.holes) {
            int hole_edge_count;
            in >> hole_edge_count;
            hole.edge_indicies.resize(hole_edge_count);
            for (auto& edge_index : hole.edge_indicies) {
                in >> edge_index;
            }
        }
    }

    return building;
}

Building transform(const Building& b, const Matrix& m) {
    auto r = b;
    for (auto& v : r.vertices) {
        v = transform(v, m);
    }
    return r;
}

Building translate(const Building& b, const Point& delta) {
    auto r = b;
    for (auto& v : r.vertices) {
        v = translate(v, delta);
    }
    return r;
}

std::vector<int> GetSortedVertexMap(const Building& b) {
    std::vector<int> vertex_map(b.vertices.size());

    std::iota(vertex_map.begin(), vertex_map.end(), 0);

    std::sort(vertex_map.begin(), vertex_map.end(),
        [&](int lhs, int rhs) {
            return b.vertices[lhs] < b.vertices[rhs];
        }
    );

    return vertex_map;
}

void UniqueVertices(std::vector<int>& vertex_map, const Building& b) {
    auto last = std::unique(vertex_map.begin(), vertex_map.end(),
        [&](int lhs, int rhs) {
            return b.vertices[lhs] < b.vertices[rhs];
        }
    );
    vertex_map.erase(last, vertex_map.end());
}

bool isSame(const Building& b1, const Building& b2) {
    auto b1_vertex_map = GetSortedVertexMap(b1);
    auto b2_vertex_map = GetSortedVertexMap(b2);

    UniqueVertices(b1_vertex_map, b1);
    UniqueVertices(b2_vertex_map, b2);

    // if we have different set of vertices, they can't be izomo
    if (!std::equal(
        b1_vertex_map.begin(), b1_vertex_map.end(),
        b2_vertex_map.begin(), b2_vertex_map.end(),
        [&](int lhs, int rhs) {
            return b1.vertices[lhs] == b2.vertices[rhs];
        }))
    {
        return false;
    }

    return true;
}

bool isCongruentish(const Building& b1, const Building& b2) {
    if (b1.vertices.empty()) {
        return true;
    }

    for (int rx = 0; rx < 4; ++rx) {
        for (int ry = 0; ry < 4; ++ry) {
            for (int rz = 0; rz < 4; ++rz) {
                auto rotationMatrix = rotationMatrixFor(rx, ry, rz);
                std::cerr << rotationMatrix << std::endl;

                auto rotated_b2 = transform(b2, rotationMatrix);

                auto min_vertex_b1 = *std::min_element(
                    b1.vertices.begin(), b1.vertices.end());
                auto min_vertex_b2 = *std::min_element(
                    rotated_b2.vertices.begin(), rotated_b2.vertices.end());

                auto tb1 = translate(b1, min_vertex_b1.negate());
                auto tb2 = translate(rotated_b2, min_vertex_b2.negate());

                if (isSame(tb1, tb2)) {
                    return true;
                }
            }
        }
    }
    return false;
}

int main() {
    int building_count;
    std::cin >> building_count;
    std::vector<Building> buildings(building_count);

    for (auto& building : buildings) {
        building = read(std::cin);
    }

    std::vector<int> good_indexes;
    for (int i = 1; i < buildings.size(); ++i) {
        if (isCongruentish(buildings[0], buildings[i])) {
            good_indexes.push_back(i);
        }
    }

    for (auto i : good_indexes) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
}
