#include <iostream>
#include <vector>
#include <array>
#include <tuple>
#include <cassert>
#include <numeric>
#include <algorithm>
#include <sstream>
#include <set>
#include <chrono>

#include <boost/lexical_cast.hpp>
#include <boost/functional/hash.hpp>

using Clock = std::chrono::steady_clock;

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

struct Separator {};

std::ostream& operator<<(std::ostream& os, const Separator&) {
    return os << " | ";
}

std::size_t hash_value(const Point& p) {
    size_t seed = p.x;
    boost::hash_combine(seed, p.y);
    boost::hash_combine(seed, p.z);
    return seed;
}

std::size_t hash_value(Separator) {
    static const int sok = 10'000'000;
    return hash_value(Point{sok, sok, sok});
}

bool operator<(const Point& lhs, const Point& rhs) {
    return
        std::tie(lhs.x, lhs.y, lhs.z) <
        std::tie(rhs.x, rhs.y, rhs.z);
}

bool operator>(const Point& lhs, const Point& rhs) {
    return rhs < lhs;
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

bool operator<(const Matrix& a, const Matrix& b) {
    return
        std::tie(a.a, a.b, a.c, a.d, a.e, a.f, a.g, a.h, a.i) <
        std::tie(b.a, b.b, b.c, b.d, b.e, b.f, b.g, b.h, b.i);
}

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

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
    for (const T& t : v) {
        os << t << " ";
    }
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

    // non-state
    std::vector<int> sorted_edge_indicies;
};

struct Face {
    std::vector<int> edge_indicies;
    std::vector<Hole> holes;

    // non-state
    std::vector<int> sorted_edge_indicies;
    std::vector<int> sorted_hole_indicies;
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
        --e.start_index;
        --e.end_index;
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
            --edge_index;
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
                --edge_index;
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

void OrderEdges(Building& b) {
    for (auto& e : b.edges) {
        if (b.vertices[e.start_index] > b.vertices[e.end_index]) {
            std::swap(e.start_index, e.end_index);
        }
    }
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

std::vector<int> GetSortedEdgeMap(
    const Building& b,
    const std::vector<int>& edge_map)
{
    std::vector<int> sorted_edge_map = edge_map;

    std::sort(sorted_edge_map.begin(), sorted_edge_map.end(),
        [&](int lhs, int rhs) {
            return
                std::tie(
                    b.vertices[b.edges[lhs].start_index],
                    b.vertices[b.edges[lhs].end_index]
                ) <
                std::tie(
                    b.vertices[b.edges[rhs].start_index],
                    b.vertices[b.edges[rhs].end_index]
                );
        }
    );
    return sorted_edge_map;
}

std::vector<int> GetSortedEdgeMapForBuilding(const Building& b) {
    std::vector<int> edge_map(b.edges.size());

    std::iota(edge_map.begin(), edge_map.end(), 0);

    return GetSortedEdgeMap(b, edge_map);
}


bool isVerticesSame(const Building& b1, const Building& b2) {
    auto b1_vertex_map = GetSortedVertexMap(b1);
    auto b2_vertex_map = GetSortedVertexMap(b2);

    // if we have different set of vertices, they can't be izomo
    return std::equal(
        b1_vertex_map.begin(), b1_vertex_map.end(),
        b2_vertex_map.begin(), b2_vertex_map.end(),
        [&](int lhs, int rhs) {
            return b1.vertices[lhs] == b2.vertices[rhs];
        });
}

bool isEdgesSame(const Building& b1, const Building& b2) {
    auto b1_edge_map = GetSortedEdgeMapForBuilding(b1);
    auto b2_edge_map = GetSortedEdgeMapForBuilding(b2);

    // if we have different set of edges, they can't be izomo
    return std::equal(
        b1_edge_map.begin(), b1_edge_map.end(),
        b2_edge_map.begin(), b2_edge_map.end(),
        [&](int lhs, int rhs) {
            return
                std::tie(
                    b1.vertices[b1.edges[lhs].start_index],
                    b1.vertices[b1.edges[lhs].end_index]
                ) ==
                std::tie(
                    b2.vertices[b2.edges[rhs].start_index],
                    b2.vertices[b2.edges[rhs].end_index]
                );
        });
}

void SetupFaceIndicies(Building& b) {
    // setup hole and face edge indicies
    for (auto& face : b.faces) {
        face.sorted_edge_indicies = GetSortedEdgeMap(b, face.edge_indicies);
        for (auto& hole : face.holes) {
            hole.sorted_edge_indicies = GetSortedEdgeMap(b, hole.edge_indicies);
        }
    }

    // setup hole indicies
    for (auto& face : b.faces) {
        face.sorted_hole_indicies.resize(face.holes.size());

        std::iota(face.sorted_hole_indicies.begin(), face.sorted_hole_indicies.end(), 0);

        std::sort(face.sorted_hole_indicies.begin(), face.sorted_hole_indicies.end(),
            [&](int lhs, int rhs) {
                auto& lh = face.holes[lhs];
                auto& rh = face.holes[rhs];

                if (lh.sorted_edge_indicies.size() != rh.sorted_edge_indicies.size()) {
                    return lh.sorted_edge_indicies.size() < rh.sorted_edge_indicies.size();
                }

                for (std::size_t i = 0; i < lh.sorted_edge_indicies.size(); ++i) {
                    auto& le = b.edges[lh.sorted_edge_indicies[i]];
                    auto& re = b.edges[rh.sorted_edge_indicies[i]];

                    auto lt = std::tie(b.vertices[le.start_index], b.vertices[le.end_index]);
                    auto rt = std::tie(b.vertices[re.start_index], b.vertices[re.end_index]);

                    if (lt != rt) {
                        return lt < rt;
                    }
                }
                return false; // equal
            }
        );
    }
}

template<typename U>
void Serialize(std::size_t& seed, const U& v) {
    boost::hash_combine(seed, v);
}

template<typename U>
void Serialize(std::string& seed, const U& v) {
    seed += boost::lexical_cast<std::string>(v);
}

template<typename T>
std::vector<T> HashFaces(const Building& b) {
    std::vector<T> face_hashes;
    face_hashes.reserve(b.faces.size());

    for (auto& face : b.faces) {
        T seed{};
        for (int ei : face.sorted_edge_indicies) {
            Serialize(seed, b.vertices[b.edges[ei].start_index]);
            Serialize(seed, b.vertices[b.edges[ei].end_index]);
        }
        Serialize(seed, Separator{});
        for (int hi : face.sorted_hole_indicies) {
            auto& hole = face.holes[hi];
            for (int ei : hole.sorted_edge_indicies) {
                Serialize(seed, b.vertices[b.edges[ei].start_index]);
                Serialize(seed, b.vertices[b.edges[ei].end_index]);
            }
            Serialize(seed, Separator{});
        }
        face_hashes.push_back(seed);
    }
    return face_hashes;
}

bool isFacesSame(Building& b1, Building& b2) {
    SetupFaceIndicies(b1);
    SetupFaceIndicies(b2);

#if 1
    auto hashes_b1 = HashFaces<std::size_t>(b1);
    auto hashes_b2 = HashFaces<std::size_t>(b2);
#else
    auto hashes_b1 = HashFaces<std::string>(b1);
    auto hashes_b2 = HashFaces<std::string>(b2);

    for (int i = 0; i < hashes_b1.size(); ++i) {
        std::cerr << "Hash(1): " << hashes_b1[i] << std::endl;
        std::cerr << "Hash(2): " << hashes_b2[i] << std::endl;
    }

#endif

    std::sort(hashes_b1.begin(), hashes_b1.end());
    std::sort(hashes_b2.begin(), hashes_b2.end());

    return hashes_b1 == hashes_b2;
}

bool isSame(Building b1, Building b2) {
    if (!isVerticesSame(b1, b2)) {
        std::cerr << "Vertex check failed" << std::endl;
        return false;
    }
    if (!isEdgesSame(b1, b2)) {
        std::cerr << "Edge check failed" << std::endl;
        return false;
    }
    if (!isFacesSame(b1, b2)) {
        std::cerr << "Face check failed" << std::endl;
        return false;
    }

    return true;
}

std::vector<Matrix> rotationMatrices() {
    std::vector<Matrix> result;
    for (int ry = 0; ry < 4; ++ry) {
        for (int rz = 0; rz < 4; ++rz) {
            result.push_back(rotationMatrixFor(0, ry, rz));
        }
    }
    for (int rz = 0; rz < 4; ++rz) {
        result.push_back(rotationMatrixFor(1, 0, rz));
    }

    for (int rz = 0; rz < 4; ++rz) {
        result.push_back(rotationMatrixFor(1, 2, rz));
    }

    assert(result.size() == 24);
    return result;
}

bool isCongruentish(const Building& b1, const Building& b2) {
    if (b1.vertices.size() != b2.vertices.size()) {
        return false;
    }
    if (b1.vertices.empty()) {
        return true;
    }
    if (b1.edges.size() != b2.edges.size()) {
        return false;
    }
    if (b1.faces.size() != b2.faces.size()) {
        return false;
    }

    // preprocess b1
    auto min_vertex_b1 = *std::min_element(
        b1.vertices.begin(), b1.vertices.end());

    auto tb1 = translate(b1, min_vertex_b1.negate());
    OrderEdges(tb1);

    for (const Matrix& rotationMatrix : rotationMatrices()) {
        auto rotated_b2 = transform(b2, rotationMatrix);

        auto min_vertex_b2 = *std::min_element(
            rotated_b2.vertices.begin(), rotated_b2.vertices.end());

        auto tb2 = translate(rotated_b2, min_vertex_b2.negate());

        OrderEdges(tb2);

        if (isSame(tb1, tb2)) {
            return true;
        }
    }
    return false;
}

int main() {
    auto start = Clock::now();
    std::cerr << "------- STARTING --------" << std::endl;

    int building_count;
    std::cin >> building_count;
    std::vector<Building> buildings(building_count);

    for (auto& building : buildings) {
        building = read(std::cin);
    }
    auto read_end = Clock::now();

    std::cerr << "Reading time: " <<
        std::chrono::duration_cast<std::chrono::milliseconds>(read_end - start).count()
        << std::endl;

    std::vector<int> good_indexes;
    for (std::size_t i = 1; i < buildings.size(); ++i) {
        if (isCongruentish(buildings[0], buildings[i])) {
            good_indexes.push_back(i);
        }
    }

    auto calculate_end = Clock::now();
    std::cerr << "Calculating time: " <<
        std::chrono::duration_cast<std::chrono::milliseconds>(calculate_end - read_end).count()
        << std::endl;

    for (auto i : good_indexes) {
        std::cout << i+1 << " ";
    }
    std::cout << std::endl;
}
