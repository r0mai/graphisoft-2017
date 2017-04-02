#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <SFML/Graphics.hpp>
// #include <SFML/Font.hpp>

#define GUI_ENABLED 1
#include "main.cpp"


struct App {
    sf::RenderWindow window;
    sf::Font font;
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

void HandleKeypress(App& app, const sf::Event::KeyEvent& ev) {
    switch (ev.code) {
        case sf::Keyboard::R:
            app.grid.InitRays();
            break;
        case sf::Keyboard::Space:
            app.grid.TraceNext();
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
            case sf::Event::KeyPressed:
                HandleKeypress(app, event.key);
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

sf::Color TileColor(Field field) {
    switch (field) {
        case Field::kEdge: return sf::Color(0x92, 0xca, 0x30);
        case Field::kFree: return sf::Color(0xff, 0xff, 0xf0);
        case Field::kWall: return sf::Color(0x66, 0x50, 0x66);
        case Field::kStop: return sf::Color(0xff, 0xf0, 0xa0);
    }
}

sf::ConvexShape CreateTile(const sf::Vector2f& pos) {
    sf::Vector2f dx(0.5f, 0);
    sf::Vector2f dy(0, sqrt(3.0f) / 2.0f);
    sf::Vector2f cx(1.0f, 0);
    float s = 0.98;

    dx *= s;
    dy *= s;
    cx *= s;

    sf::ConvexShape tile;
    tile.setPointCount(6);
    tile.setPoint(0, dx - dy);
    tile.setPoint(1, cx);
    tile.setPoint(2, dx + dy);
    tile.setPoint(3, -dx + dy);
    tile.setPoint(4, -cx);
    tile.setPoint(5, -dx - dy);
    tile.setPosition(pos);

    return tile;
}

sf::CircleShape CreateDot(const sf::Vector2f& pos, Dir dir) {
    sf::CircleShape circ;
    float r = 0.05f;

    circ.setRadius(r);
    circ.setOrigin(-0.65, r);
    circ.rotate(-90 + int(dir) * 60);
    circ.setPosition(pos);

    return circ;
}

sf::Text CreateText(const sf::Vector2f& pos, const sf::Font& font,
        const std::string& str)
{
    sf::Text text(str, font, 48);
    text.setFillColor(sf::Color(0, 0, 0, 0x60));

    auto box = text.getLocalBounds();
    auto scale = 1.0f / 96.0f;
    text.setPosition(
        pos.x + -(box.left + box.width) * 0.5f * scale,
        pos.y + -(12 + box.top + box.height) * 0.5f * scale);
    text.setScale(scale, scale);

    return text;
}

std::string ToString(int n) {
    std::stringstream ss;
    ss << n;
    return ss.str();
}

void Draw(App& app) {
    auto& window = app.window;
    auto size = app.grid.Size();
    auto dy = sqrt(3.0f);

    window.clear();
    for (int row = -1; row <= size.row; ++row) {
        for (int col = -1; col <= size.col; ++col) {
            auto off = col % 2 ? 0.5f : 0.0f;
            sf::Vector2f pos(col * 1.5f, (row + off) * dy);
            auto tile = CreateTile(pos);

            auto field = app.grid.GetField({row, col});
            auto ray = app.grid.GetRay({row, col});
            tile.setFillColor(TileColor(field));
            window.draw(tile);
            if (ray.bounce != Ray::kUnreached) {
                auto text = CreateText(pos, app.font, ToString(ray.bounce));
                window.draw(text);
                for (auto dir : ray.GetUnusedVector()) {
                    auto dot = CreateDot(pos, dir);
                    dot.setFillColor(sf::Color::Black);
                    window.draw(dot);
                }
            }
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

void LoadFont(App& app) {
    std::vector<uint8_t> data({
        #include "font/trebuchet_ms_ttf.h"
    });
    app.font.loadFromMemory(data.data(), data.size());
}


int main() {
    App app;
    app.grid.FromStream(std::cin);

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    app.window.create(
        sf::VideoMode(1600, 1000), "Labirintus",
        sf::Style::Default,
        settings
    );

    LoadFont(app);
    AdjustView(app);
    Run(app);

    return 0;
}
