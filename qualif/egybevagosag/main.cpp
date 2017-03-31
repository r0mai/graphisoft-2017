#include <iostream>
#include <vector>

struct Point {
    Point() = default;
    Point(int x, int y, int z) : x(x), y(y), z(z) {}

    int x;
    int y;
    int z;
};

struct Edge {
    Edge() = default;
    Edge(Point start, Point end) : start(start), end(end) {}

    Point start;
    Point end;
};

struct Hole {
    std::vector<Edge> edges;
};

struct Face {
    std::vector<Edge> edges;
    std::vector<Hole> holes;
};

struct Building {
    std::vector<Face> faces;
};

Building read(std::istream& in) {
    int vertex_count;
    in >> vertex_count;

    std::vector<Point> vertices(vertex_count);
    for (auto& p : vertices) {
        in >> p.x >> p.y >> p.z;
    }

    int edge_count;
    in >> edge_count;

    std::vector<Edge> edges(edge_count);
    for (auto& e : edges) {
        int is, ie;
        in >> is >> ie;
        e.start = vertices[is];
        e.end = vertices[ie];
    }

    int face_count;
    in >> face_count;
    std::vector<Face> faces(face_count);

    for (auto& face : faces) {
        int face_edge_count;
        in >> face_edge_count;
        face.edges.resize(face_edge_count);

        for (auto& edge : face.edges) {
            int i;
            in >> i;
            edge = edges[i];
        }

        int hole_count;
        in >> hole_count;
        face.holes.resize(hole_count);

        for (auto& hole : face.holes) {
            int hole_edge_count;
            in >> hole_edge_count;
            hole.edges.resize(hole_edge_count);
            for (auto& edge : hole.edges) {
                int i;
                in >> i;
                edge = edges[i];
            }
        }
    }

    Building building;
    building.faces = faces;
    return building;
}

bool isCongruentish(const Building& b1, const Building& b2) {
    return false;
}

int main() {
    auto b1 = read(std::cin);
    auto b2 = read(std::cin);

    std::cout << (isCongruentish(b1, b2) ? "TRUE" : "FALSE") << std::endl;
}
