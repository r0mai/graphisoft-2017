#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cmath>
#include <SFML/Graphics.hpp>

#define GUI_ENABLED 1
#include "main.cpp"


struct App {
    sf::RenderWindow window;
    HexGrid grid;
};


void AdjustView(App& app) {
    auto size = app.window.getSize();
    auto rows = app.grid.Rows() + 2;
    auto cols = app.grid.Cols() + 2;

    float q = sqrt(3.0f);
    float w = 0.5f + cols * 1.5f;
    float h = q * (rows + 0.5f);

    if (size.x * h > size.y * w) {
        w = size.x * h / size.y;
    } else {
        h = size.y * w / size.x;
    }

    app.window.setView(sf::View(sf::FloatRect(-2.5f, -1.5f * q, w, h)));
}


void HandleEvents(App& app) {
    sf::Event event;
    while (app.window.pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed:
                app.window.close();
                break;
            case sf::Event::Resized: {
                AdjustView(app);
                break;
            }
            default:
                break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}


sf::ConvexShape CreateTile() {
    sf::Vector2f dx(0.5f, 0);
    sf::Vector2f dy(0, sqrt(3.0f) / 2.0f);
    sf::Vector2f cx(1.0f, 0);

    sf::ConvexShape tile;
    tile.setPointCount(6);
    tile.setPoint(0, dx - dy);
    tile.setPoint(1, cx);
    tile.setPoint(2, dx + dy);
    tile.setPoint(3, -dx + dy);
    tile.setPoint(4, -cx);
    tile.setPoint(5, -dx - dy);

    return tile;
}

sf::Color TileColor(Field field) {
    switch (field) {
        case Field::kEdge: return sf::Color(0x92, 0xca, 0x30);
        case Field::kFree: return sf::Color(0xff, 0xff, 0xf0);
        case Field::kWall: return sf::Color(0x66, 0x50, 0x66);
        case Field::kStop: return sf::Color(0xff, 0xf0, 0xa0);
    }
}


void Draw(App& app) {
    auto& window = app.window;
    auto size = app.grid.Size();
    auto tile = CreateTile();
    auto dy = sqrt(3.0f);
    float s = 1;
    float ts = s * 0.98;

    tile.setScale(ts, ts);

    window.clear();
    for (int row = -1; row <= size.row; ++row) {
        for (int col = -1; col <= size.col; ++col) {
            auto off = col % 2 ? 0.5f : 0.0f;
            tile.setPosition(
                col * s * 1.5f,
                (row + off) * s * dy);
            tile.setFillColor(TileColor(app.grid.GetField(Pos(row, col))));
            window.draw(tile);
        }
    }

    window.display();
}


void Run(App& app) {
    while (app.window.isOpen()) {
        HandleEvents(app);
        Draw(app);
    }
}



int main() {
    App app;
    app.grid.FromStream(std::cin);
    app.window.create(sf::VideoMode(1600, 1000), "Labirintus");

    AdjustView(app);
    Run(app);

    return 0;
}
