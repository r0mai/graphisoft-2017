#include <cassert>
#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <cstdlib>
#include <SFML/Graphics.hpp>
#include "Point.h"
#include "Grid.h"

struct App {
    sf::RenderWindow window;
    sf::Font font;
    Grid grid;
};


void AdjustView(App& app) {
    auto size = app.window.getSize();
    float h = app.grid.GetHeight() + 2;
    float w = app.grid.GetWidth() + 2;

    if (size.x * h > size.y * w) {
        w = size.x * h / size.y;
    } else {
        h = size.y * w / size.x;
    }

    app.window.setView(sf::View(sf::FloatRect(-1, -1, w, h)));
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
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

sf::RectangleShape CreateTile(const sf::Vector2f& pos) {
    float size = 0.98f;
    sf::RectangleShape shape(sf::Vector2f{size, size});
    shape.setPosition(pos);
    return shape;
}

sf::ConvexShape CreateRoute(const sf::Vector2f& pos, int tile) {
    float pad = 0.3f;
    float size = 0.98f;

    sf::Vector2f dx(size, 0.f);
    sf::Vector2f dy(0.f, size);

    auto dx0 = dx * 0.f;
    auto dx1 = dx * 1.f;
    auto dxm0 = dx * pad;
    auto dxm1 = dx * (1.f - pad);

    auto dy0 = dy * 0.f;
    auto dy1 = dy * 1.f;
    auto dym0 = dy * pad;
    auto dym1 = dy * (1.f - pad);

    int count = 0;
    count += !!(tile & 0x1);
    count += !!(tile & 0x2);
    count += !!(tile & 0x4);
    count += !!(tile & 0x8);

    sf::ConvexShape shape;
    shape.setPointCount(count * 2 + 4);

    int k = 0;
    shape.setPoint(k++, dxm1 + dym0);
    if (!!(tile & 0x1)) {
        shape.setPoint(k++, dxm1 + dy0);
        shape.setPoint(k++, dxm0 + dy0);
    }

    shape.setPoint(k++, dxm0 + dym0);
    if (!!(tile & 0x2)) {
        shape.setPoint(k++, dx0 + dym0);
        shape.setPoint(k++, dx0 + dym1);
    }

    shape.setPoint(k++, dxm0 + dym1);
    if (!!(tile & 0x4)) {
        shape.setPoint(k++, dxm0 + dy1);
        shape.setPoint(k++, dxm1 + dy1);
    }

    shape.setPoint(k++, dxm1 + dym1);
    if (!!(tile & 0x8)) {
        shape.setPoint(k++, dx1 + dym1);
        shape.setPoint(k++, dx1 + dym0);
    }
    shape.setPosition(pos);

    return shape;
}


void Draw(App& app) {
    auto& window = app.window;
    auto size = app.grid.GetSize();

    window.clear();

    for (int y = 0; y < size.y; ++y) {
        for (int x = 0; x < size.x; ++x) {
            auto tile = CreateTile(sf::Vector2f(x, y));
            auto route = CreateRoute(sf::Vector2f(x, y), app.grid.At(x, y));
            tile.setOutlineThickness(0.01);
            tile.setOutlineColor(sf::Color(0x40, 0x30, 0x40));
            tile.setFillColor(sf::Color(0x66, 0x50, 0x66));
            route.setFillColor(sf::Color(0xf0, 0xf0, 0xe0));

            window.draw(tile);
            window.draw(route);
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
    app.grid.Init(14, 8, 0, 0);
    app.grid.Randomize();

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    app.window.create(
        sf::VideoMode(1600, 1000), "Labirintus",
        sf::Style::Default,
        settings
    );

    AdjustView(app);
    Run(app);

    return 0;
}
