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
#include "Util.h"


struct App {
    sf::RenderWindow window;
    Grid grid;

    int extra = 3;
    Point hover;
};


void AdjustView(App& app) {
    auto size = app.window.getSize();
    float w0 = app.grid.Width();
    float h0 = app.grid.Height();
    float w = w0 + 3.5;
    float h = h0 + 2.5;

    if (size.x * h > size.y * w) {
        w = size.x * h / size.y;
    } else {
        h = size.y * w / size.x;
    }

    float dx = (w - w0 - 1.5) / 2;
    float dy = (h - h0) / 2;
    app.window.setView(sf::View(sf::FloatRect(-dx, -dy, w, h)));
}

void HandleKeypress(App& app, const sf::Event::KeyEvent& ev) {
    switch (ev.code) {
        case sf::Keyboard::A:
            app.extra = RotateLeft(app.extra);
            break;
        case sf::Keyboard::D:
            app.extra = RotateRight(app.extra);
            break;
        default:
            break;
    }
}


sf::Vector2f windowToView(App& app, const Point& pos) {
    auto size = app.window.getSize();
    auto view = app.window.getView();
    auto view_size = view.getSize();
    auto view_center = view.getCenter();

    auto offset = view_center - view_size * 0.5f;

    auto mx = (float(pos.x) / size.x) * view_size.x + offset.x;
    auto my = (float(pos.y) / size.y) * view_size.y + offset.y;
    return {mx, my};
}

Point roundToTile(const sf::Vector2f& pos) {
    return Point(floor(pos.x), floor(pos.y));
}

void HandleMouseMoved(App& app, const sf::Event::MouseMoveEvent& ev) {
    auto pos = roundToTile(windowToView(app, {ev.x, ev.y}));
    app.hover = pos;
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
            case sf::Event::MouseMoved:
                HandleMouseMoved(app, event.mouseMove);
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
    dot.setFillColor(active ? sf::Color::Red : sf::Color::Black);
    app.window.draw(dot);
}


void Draw(App& app) {
    auto& window = app.window;
    const auto& hover = app.hover;
    auto size = app.grid.Size();

    window.clear(sf::Color(0x33, 0x33, 0x33));

    for (int y = 0; y < size.y; ++y) {
        for (int x = 0; x < size.x; ++x) {
            DrawTile(app, sf::Vector2f(x, y), app.grid.At(x, y));
        }
    }

    for (int y = 0; y < size.y; ++y) {
        bool active0 = false;
        bool active1 = false;
        if (hover.y == y) {
            active0 = hover.x == -1;
            active1 = hover.x == size.x;
        }

        DrawDot(app, sf::Vector2f(-1, y), active0);
        DrawDot(app, sf::Vector2f(size.x, y), active1);
    }

    for (int x = 0; x < size.x; ++x) {
        bool active0 = false;
        bool active1 = false;
        if (hover.x == x) {
            active0 = hover.y == -1;
            active1 = hover.y == size.y;
        }

        DrawDot(app, sf::Vector2f(x, -1), active0);
        DrawDot(app, sf::Vector2f(x, size.y), active1);
    }

    DrawTile(app, sf::Vector2f(size.x + 1, -1), app.extra);
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
