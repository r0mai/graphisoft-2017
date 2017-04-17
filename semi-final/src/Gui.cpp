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
#include "FloodFill.h"
#include "EagerTaxicab.h"

// -> {Push, Move} func(Grid, player, target, extra)

struct Game {
	Grid grid;
	std::vector<Field> extras;
	int tick = -1;
	int player = -1;

	int players = 0;
	int displays = 0;
};

enum class State {
	kPush,
	kMove,
	kDone
};

struct App {
	sf::RenderWindow window;
	Matrix<int> colors;
	Point hover;

	State state;

	Grid grid;
	Field extra;
	Field field; // saved when push is done
	int self = 0;
	int target = 0;

	Point push;
	Point move;
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
		case sf::Keyboard::Space:
			if (app.state == State::kMove) {
				app.state = State::kDone;
			}
			break;
		case sf::Keyboard::A:
			app.extra = RotateLeft(app.extra);
			break;
		case sf::Keyboard::D:
			app.extra = RotateRight(app.extra);
			break;
		case sf::Keyboard::P:
			// if (app.state == State::kPush) {
			// 	EagerTaxicab ai(app.grid, app.self, app.target, app.extra);
			// 	auto response = ai.GetResponse();
			// 	app.push = response.push.direction;
			// 	app.move = response.move.target;
			// 	app.field = response.push.field;
			// 	app.state = State::kDone;
			// }
		default:
			break;
	}
}

sf::Vector2f WindowToView(App& app, const Point& pos) {
	auto size = app.window.getSize();
	auto view = app.window.getView();
	auto view_size = view.getSize();
	auto view_center = view.getCenter();

	auto offset = view_center - view_size * 0.5f;

	auto mx = (float(pos.x) / size.x) * view_size.x + offset.x;
	auto my = (float(pos.y) / size.y) * view_size.y + offset.y;
	return {mx, my};
}

Point RoundToTile(const sf::Vector2f& pos) {
	return Point(floor(pos.x), floor(pos.y));
}

bool IsEdge(const App& app, const Point& pos) {
	auto size = app.grid.Size();
	bool x_edge = pos.x == -1 || pos.x == size.x;
	bool y_edge = pos.y == -1 || pos.y == size.y;

	bool x_in = pos.x >= 0 && pos.x < size.x;
	bool y_in = pos.y >= 0 && pos.y < size.y;

	return (x_edge && y_in) || (y_edge && x_in);
}

bool IsInside(const App& app, const Point& pos) {
	auto size = app.grid.Size();
	bool x_in = pos.x >= 0 && pos.x < size.x;
	bool y_in = pos.y >= 0 && pos.y < size.y;

	return x_in && y_in;
}

int NextDisplay(const Grid& grid) {
	int index = 0;
	for (const auto& pos : grid.Displays()) {
		if (IsValid(pos)) {
			return index;
		}
		++index;
	}
	return -1;
}

void ResetColors(App& app) {
	app.colors = Matrix<int>(app.grid.Width(), app.grid.Height(), 0);
}

void UpdateColors(App& app) {
	auto pos = app.grid.Positions()[app.self];
	app.colors = FloodFill(app.grid.Fields(), pos);
}

void HandleMouseMoved(App& app, const sf::Event::MouseMoveEvent& ev) {
	auto pos = RoundToTile(WindowToView(app, {ev.x, ev.y}));
	app.hover = pos;
}

void HandleMousePressed(App& app, const sf::Event::MouseButtonEvent& ev) {
	auto pos = RoundToTile(WindowToView(app, {ev.x, ev.y}));
	if (app.state == State::kPush && IsEdge(app, pos)) {
		app.state = State::kMove;
		app.push = pos;
		app.field = app.extra;
		app.extra = app.grid.Push(pos, app.extra);
		UpdateColors(app);
	} else if (app.state == State::kMove && IsInside(app, pos) &&
		app.colors.At(pos) != 0)
	{
		app.state = State::kDone;
		app.move = pos;
		app.grid.UpdatePosition(0, pos);
		ResetColors(app);
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
			case sf::Event::MouseMoved:
				HandleMouseMoved(app, event.mouseMove);
				break;
            case sf::Event::MouseButtonPressed:
                HandleMousePressed(app, event.mouseButton);
                break;
			default:
				break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}

sf::CircleShape CreateDot(const sf::Vector2f& pos, float r=0.05f) {
	sf::CircleShape circ;
	circ.setRadius(r);
	circ.setOrigin(r - 0.5f, r - 0.5f);
	circ.setPosition(pos);

	return circ;
}

sf::ConvexShape CreateDiamond(const sf::Vector2f& pos) {
	float size = 0.12f;

	sf::Vector2f dx(size, 0.f);
	sf::Vector2f dy(0.f, size);
	sf::ConvexShape shape;
	shape.setPointCount(5);

	int k = 0;
	shape.setPoint(k++, dx * 0.5f -dy * 0.5f);
	shape.setPoint(k++, -dx * 0.5f -dy * 0.5f);
	shape.setPoint(k++, -dx);
	shape.setPoint(k++, dy);
	shape.setPoint(k++, dx);

	shape.setPosition(pos);
	shape.setOrigin(-0.5f, -0.5f);

	return shape;
}

sf::ConvexShape CreateSquare(const sf::Vector2f& pos) {
	float size = 0.13f;

	sf::Vector2f dx(size, 0.f);
	sf::Vector2f dy(0.f, size);
	sf::ConvexShape shape;
	shape.setPointCount(4);

	int k = 0;
	shape.setPoint(k++, -dx - dy);
	shape.setPoint(k++, -dx + dy);
	shape.setPoint(k++, dx + dy);
	shape.setPoint(k++, dx - dy);

	shape.setPosition(pos);
	shape.setOrigin(-0.5f, -0.5f);

	return shape;
}


sf::RectangleShape CreateTile(const sf::Vector2f& pos) {
	float size = 0.98f;
	sf::RectangleShape shape(sf::Vector2f{size, size});
	shape.setPosition(pos);
	shape.setOrigin(-0.01f, -0.01f);
	return shape;
}

sf::ConvexShape CreateRoute(const sf::Vector2f& pos, Field tile) {
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
	shape.setOrigin(-0.01f, -0.01f);

	return shape;
}


void DrawTile(App& app, const sf::Vector2f& pos, Field tile, int color_id=0) {
	auto base = CreateTile(pos);
	auto route = CreateRoute(pos, tile);
	base.setOutlineThickness(0.01f);
	base.setOutlineColor(sf::Color(0x40, 0x30, 0x40));
	base.setFillColor(sf::Color(0x66, 0x50, 0x66));

	sf::Color color = sf::Color(0xf0, 0xf0, 0xe0);
	switch (color_id) {
		case 1:
			color = sf::Color(0xff, 0xc4, 0x14);
			break;
		default:
			break;
	}
	route.setFillColor(color);

	app.window.draw(base);
	app.window.draw(route);
}


void DrawDot(App& app, const sf::Vector2f& pos, bool active=false) {
	auto dot = CreateDot(pos);
	dot.setFillColor(active ? sf::Color::Red : sf::Color::Black);
	app.window.draw(dot);
}

void DrawPrincesses(App& app) {
	static const sf::Color colors[] = {
		sf::Color(0xff, 0, 0),
		sf::Color(0x20, 0xcf, 0),
		sf::Color(0xff, 0xef, 0x30),
		sf::Color(0x60, 0x40, 0xff),
	};

	std::map<Point, int> pos_map;

	int i = 0;
	for (const auto& pos : app.grid.Positions()) {
		pos_map[pos] |= (1 << i);
		++i;
	}

	sf::Vector2f offset{-0.03f, -0.03f};
	for (const auto& p : pos_map) {
		const auto& pos = p.first;
		bool self = !!(p.second & (1<<app.self));

		int k = -1;
		for (int i = 0; i < 4; ++i) {
			k += !!(p.second & (1<<i));
		}

		for (int i = 0; i < 4; ++i) {
			if (!!(p.second & (1<<i)) && i != app.self) {
				auto color = colors[i];
				auto dot = CreateDiamond(
					sf::Vector2f(pos.x, pos.y) + float(k) * offset);

				color.a = 0x80;
				dot.setOutlineThickness(-0.01f);
				dot.setOutlineColor(
					// color);
					sf::Color(0, 0, 0, 0x80));
				dot.setFillColor(
					color);
					// sf::Color(0xff, 0xff, 0xff,0x80));
				app.window.draw(dot);
				--k;
			}
		}

		if (self) {
			auto dot = CreateDiamond(sf::Vector2f(pos.x, pos.y));
			dot.setOutlineThickness(0.01f);
			dot.setOutlineColor(sf::Color(0, 0, 0, 0x80));
			dot.setFillColor(colors[app.self]);
			app.window.draw(dot);
		}
	}

	if (1) {
		// draw on extra tile
		auto size = app.grid.Size();
		auto dot = CreateDiamond(sf::Vector2f(size.x + 1, -1));
		dot.setOutlineThickness(0.01f);
		dot.setOutlineColor(sf::Color(0, 0, 0, 0x80));
		dot.setFillColor(colors[app.self]);
		app.window.draw(dot);
	}
}

void DrawDisplays(App& app) {
	int index = 0;
	for (const auto& pos : app.grid.Displays()) {
		if (IsValid(pos)) {
			auto dot = CreateSquare(sf::Vector2f(pos.x, pos.y));
			dot.setOutlineThickness(0.01f);
			dot.setOutlineColor(sf::Color(0, 0, 0, 0x60));
			dot.setFillColor(
				index == app.target
				? sf::Color(0x83, 0xb8, 0x0b)
				: sf::Color(0xef, 0xff, 0xaf));
			app.window.draw(dot);
		}
		++index;
	}
}

void Draw(App& app) {
	auto& window = app.window;
	const auto& hover = app.hover;
	auto size = app.grid.Size();
	auto is_push = app.state == State::kPush;

	window.clear(sf::Color(0x33, 0x33, 0x33));

	for (int y = 0; y < size.y; ++y) {
		for (int x = 0; x < size.x; ++x) {
			auto color_id = app.colors.At(x, y);
			DrawTile(app, sf::Vector2f(x, y), app.grid.At(x, y), color_id);
		}
	}

	for (int y = 0; y < size.y; ++y) {
		bool active0 = false;
		bool active1 = false;
		if (is_push && hover.y == y) {
			active0 = hover.x == -1;
			active1 = hover.x == size.x;
		}

		DrawDot(app, sf::Vector2f(-1, y), active0);
		DrawDot(app, sf::Vector2f(size.x, y), active1);
	}

	for (int x = 0; x < size.x; ++x) {
		bool active0 = false;
		bool active1 = false;
		if (is_push && hover.x == x) {
			active0 = hover.y == -1;
			active1 = hover.y == size.y;
		}

		DrawDot(app, sf::Vector2f(x, -1), active0);
		DrawDot(app, sf::Vector2f(x, size.y), active1);
	}

	DrawTile(app, sf::Vector2f(size.x + 1, -1), app.extra);

	DrawDisplays(app);
	DrawPrincesses(app);
	window.display();
}

void NextPlayer(Game& game, App& app) {
	game.player = (game.player + 1) % game.players;
	if (game.player == 0) {
		++game.tick;
	}

	app.grid = game.grid;
	app.self = game.player;
	app.target = NextDisplay(app.grid);
	app.state = State::kPush;
	app.extra = game.extras[game.player];
	app.push = {};
	app.move = {};
	ResetColors(app);
}

void ApplyMove(Game& game, App& app) {
	auto& extra = game.extras[game.player];
	extra = game.grid.Push(app.push, app.field);
	if (IsValid(app.move)) {
		// TODO: check if this is a valid move
		auto target = app.target;
		game.grid.UpdatePosition(game.player, app.move);
		if (target != -1 && app.move == game.grid.Displays()[app.target]) {
			game.grid.UpdateDisplay(target, {});
		}
	}
}

void Run(Game& game, App& app) {
	while (app.window.isOpen()) {
		HandleEvents(app);
		Draw(app);
		if (app.state == State::kDone) {
			ApplyMove(game, app);
			NextPlayer(game, app);
		}
	}
}

void InitGame(Game& game) {
	game.displays = 6;
	game.players = 4;
	game.grid.Init(14, 8, game.displays, game.players);
	game.grid.Randomize();
	game.extras.resize(game.players, Field(15));
}

int main() {
	srand(10371);

	Game game;
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	App app;
	app.window.create(
		sf::VideoMode(1600, 1000), "Labirintus",
		sf::Style::Default,
		settings
	);

	InitGame(game);
	NextPlayer(game, app);
	AdjustView(app);
	Run(game, app);

	return 0;
}
