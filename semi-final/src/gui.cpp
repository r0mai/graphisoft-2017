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
    float w0 = app.grid.Width();
    float h0 = app.grid.Height();
    float w = w0 + 4;
    float h = h0 + 2;

    if (size.x * h > size.y * w) {
        w = size.x * h / size.y;
    } else {
        h = size.y * w / size.x;
    }

    float dx = (w - w0 - 2) / 2;
    float dy = (h - h0) / 2;
    app.window.setView(sf::View(sf::FloatRect(-dx, -dy, w, h)));
}

void HandleKeypress(App& app, const sf::Event::KeyEvent& ev) {
    switch (ev.code) {
        case sf::Keyboard::A:
            app.grid.RotateLeft();
            break;
        case sf::Keyboard::D:
            app.grid.RotateRight();
            break;
        default:
            break;
    }
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
            case sf::Event::KeyPressed:
                HandleKeypress(app, event.key);
                break;
            default:
                break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

sf::CircleShape CreateDot(const sf::Vector2f& pos) {
    sf::CircleShape circ;
    float r = 0.05f;

    circ.setRadius(r);
    circ.setOrigin(r - 0.5f, r - 0.5f);
    circ.setPosition(pos);

    return circ;
}

sf::RectangleShape CreateTile(const sf::Vector2f& pos) {
    float size = 0.98f;
    sf::RectangleShape shape(sf::Vector2f{size, size});
    shape.setPosition(pos);
    return shape;
}

sf::ConvexShape CreateRoute(const sf::Vector2f& pos, int tile) {
    float pad = 0.33f;
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


void DrawTile(App& app, const sf::Vector2f& pos, int tile) {
    auto base = CreateTile(pos);
    auto route = CreateRoute(pos, tile);
    base.setOutlineThickness(0.01f);
    base.setOutlineColor(sf::Color(0x40, 0x30, 0x40));
    base.setFillColor(sf::Color(0x66, 0x50, 0x66));
    route.setFillColor(sf::Color(0xf0, 0xf0, 0xe0));

    app.window.draw(base);
    app.window.draw(route);
}


void DrawDot(App& app, const sf::Vector2f& pos, bool active=false) {
    auto dot = CreateDot(pos);
    dot.setFillColor(sf::Color::Black);
    // dot.setOutlineColor(sf::Color(0xee, 0xee, 0xee));
    // dot.setOutlineThickness(0.02f);
    app.window.draw(dot);
}


void Draw(App& app) {
    auto& window = app.window;
    auto size = app.grid.Size();

    window.clear(sf::Color(0x33, 0x33, 0x33));

    for (int y = 0; y < size.y; ++y) {
        for (int x = 0; x < size.x; ++x) {
            DrawTile(app, sf::Vector2f(x, y), app.grid.At(x, y));
        }
    }

    for (int y = 0; y < size.y; ++y) {
        DrawDot(app, sf::Vector2f(-1, y));
        DrawDot(app, sf::Vector2f(size.x, y));
    }

    for (int x = 0; x < size.x; ++x) {
        DrawDot(app, sf::Vector2f(x, -1));
        DrawDot(app, sf::Vector2f(x, size.y));
    }

    DrawTile(app, sf::Vector2f(size.x + 1, -1), app.grid.Extra());
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
