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

bool isCongruentish(const Building& b1, const Building& b2) {
    return false;
}

int main() {
    auto b1 = read(std::cin);
    auto b2 = read(std::cin);

    std::cout << (isCongruentish(b1, b2) ? "TRUE" : "FALSE") << std::endl;
}
