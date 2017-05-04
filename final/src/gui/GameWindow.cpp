#include "GameWindow.h"
#include "Hsv2rgb.h"
#include "EagerTaxicab.h"
#include "UpwindSailer.h"
#include "SuperFill.h"
#include <cmath>
#include <sstream>
#include <thread>
#include <chrono>

namespace {

sf::Color PlayerColor(int n) {
	static sf::Color color[] = {
		sf::Color(0xff, 0, 0),
		sf::Color(0x20, 0xcf, 0),
		sf::Color(0xff, 0xef, 0x30),
		sf::Color(0x60, 0x40, 0xff),
	};

	return color[n];
}

std::string PlayerName(int n) {
	static std::string names[] = {
		"Red",
		"Green",
		"Yellow",
		"Blue"
	};
	return names[n];
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

void DrawTile(sf::RenderTarget& rt, const sf::Vector2f& pos, Field tile,
	int color_id=0, bool blocked=false)
{
	auto base = CreateTile(pos);
	auto route = CreateRoute(pos, tile);
	base.setOutlineThickness(0.01f);
	base.setOutlineColor(sf::Color(0x40, 0x30, 0x40));

	base.setFillColor(blocked
		? sf::Color(0x80, 0x80, 0x80)
		: sf::Color(0x66, 0x50, 0x66));

	sf::Color color;
	if (color_id == 0) {
		color = sf::Color(0xf0, 0xf0, 0xe0);
	} else if (color_id == 1) {
		color = sf::Color(0xc4, 0xf9, 0x4f);
	} else if (color_id != 0) {
		color = HSVtoRGB(360 + 60 - 15 * color_id, 0.8, 1);
	}
	route.setFillColor(color);

	rt.draw(base);
	rt.draw(route);
}

void DrawDot(sf::RenderTarget& rt, const sf::Vector2f& pos, bool active=false) {
	auto dot = CreateDot(pos);
	dot.setFillColor(active ? sf::Color::Red : sf::Color::Black);
	rt.draw(dot);
}

sf::Vector2f WindowToView(sf::RenderWindow& window, const Point& pos) {
	auto size = window.getSize();
	auto view = window.getView();
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

} // namespace


GameWindow::GameWindow(Game& game)
	: game_(game)
{
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	window_.create(
		sf::VideoMode(1600, 1000), "Labirintus",
		sf::Style::Default,
		settings
	);
}

void GameWindow::Resize() {
	auto size = window_.getSize();
	float w0 = game_.GetGrid().Width();
	float h0 = game_.GetGrid().Height();
	float w = w0 + 3.5;
	float h = h0 + 2.5;

	if (size.x * h > size.y * w) {
		w = size.x * h / size.y;
	} else {
		h = size.y * w / size.x;
	}

	float dx = (w - w0 - 1.5) / 2;
	float dy = (h - h0) / 2;
	window_.setView(sf::View(sf::FloatRect(-dx, -dy, w, h)));
}

bool GameWindow::IsOpen() const {
	return window_.isOpen();
}

void GameWindow::DrawPlayers() {
	std::map<Point, int> pos_map;

	int i = 0;
	for (const auto& pos : game_.GetGrid().Positions()) {
		pos_map[pos] |= (1 << i);
		++i;
	}

	sf::Vector2f offset{-0.03f, -0.03f};
	for (const auto& p : pos_map) {
		const auto& pos = p.first;
		if (!IsValid(pos)) {
			continue;
		}

		bool self = !!(p.second & (1<<game_.GetPlayer()));
		sf::Vector2f delta{game_.RowDelta(pos.y), game_.ColDelta(pos.x)};

		int k = -1;
		for (int i = 0; i < 4; ++i) {
			k += !!(p.second & (1<<i));
		}

		for (int i = 0; i < 4; ++i) {
			if (!!(p.second & (1<<i)) && i != game_.GetPlayer()) {
				auto color = PlayerColor(i);
				auto dot = CreateDiamond(
					sf::Vector2f(pos.x, pos.y) + delta + float(k) * offset);

				color.a = 0x80;
				dot.setOutlineThickness(-0.01f);
				dot.setOutlineColor(sf::Color(0, 0, 0, 0x80));
				dot.setFillColor(color);
				window_.draw(dot);
				--k;
			}
		}

		if (self) {
			auto dp = sf::Vector2f(pos.x, pos.y) + delta + game_.PlayerDelta();
			auto dot = CreateDiamond(dp);
			dot.setOutlineThickness(0.01f);
			dot.setOutlineColor(sf::Color(0, 0, 0, 0x80));
			dot.setFillColor(PlayerColor(game_.GetPlayer()));
			window_.draw(dot);
		}
	}

	if (1) {
		// draw on extra tile
		auto size = game_.GetGrid().Size();
		auto dot = CreateDiamond(sf::Vector2f(size.x + 1, -1));
		dot.setOutlineThickness(0.01f);
		dot.setOutlineColor(sf::Color(0, 0, 0, 0x80));
		dot.setFillColor(PlayerColor(game_.GetPlayer()));
		window_.draw(dot);
	}
}

void GameWindow::DrawDisplays() {
	int index = 0;
	for (const auto& pos : game_.GetGrid().Displays()) {
		if (IsValid(pos)) {
			sf::Vector2f delta{game_.RowDelta(pos.y), game_.ColDelta(pos.x)};
			auto dot = CreateSquare(sf::Vector2f(pos.x, pos.y) + delta);
			dot.setOutlineThickness(0.01f);
			dot.setOutlineColor(sf::Color(0, 0, 0, 0x60));
			dot.setFillColor(
				index == game_.GetTarget()
				? sf::Color(0x83, 0xb8, 0x0b)
				: sf::Color(0xef, 0xff, 0xaf));
			window_.draw(dot);
		}
		++index;
	}
}

void GameWindow::Draw() {
	static State last_state = State(-1);
	if (last_state != game_.GetState()) {
		last_state = game_.GetState();
	}

	if (invalid_) {
		invalid_ = false;
		Resize();
	}

	auto size = game_.GetGrid().Size();
	auto is_push = game_.CanPush();

	window_.clear(sf::Color(0x33, 0x33, 0x33));

	const auto& grid = game_.GetGrid();
	for (int y = 0; y < size.y; ++y) {
		for (int x = 0; x < size.x; ++x) {
			auto color_id = game_.GetColor(x, y);
			sf::Vector2f pos(game_.RowDelta(y) + x, game_.ColDelta(x) + y);
			auto blocked = grid.IsBlockedX(x) && grid.IsBlockedY(y);
			DrawTile(window_, pos, grid.At(x, y), color_id, blocked);
		}
	}

	for (int y = 0; y < size.y; ++y) {
		bool active0 = false;
		bool active1 = false;
		if (is_push && hover_.y == y && !grid.IsBlockedY(y)) {
			active0 = hover_.x == -1;
			active1 = hover_.x == size.x;
		}

		DrawDot(window_, sf::Vector2f(-1, y), active0);
		DrawDot(window_, sf::Vector2f(size.x, y), active1);
	}

	for (int x = 0; x < size.x; ++x) {
		bool active0 = false;
		bool active1 = false;
		if (is_push && hover_.x == x && !grid.IsBlockedX(x)) {
			active0 = hover_.y == -1;
			active1 = hover_.y == size.y;
		}

		DrawDot(window_, sf::Vector2f(x, -1), active0);
		DrawDot(window_, sf::Vector2f(x, size.y), active1);
	}

	DrawTile(window_, sf::Vector2f(size.x + 1, -1), game_.GetExtra());

	DrawDisplays();
	DrawPlayers();

	auto elapsed = clock_.getElapsedTime().asMilliseconds();
	const auto kWait = 16;
	if (elapsed < kWait) {
		std::this_thread::sleep_for(
			std::chrono::milliseconds(kWait - elapsed));
		window_.display();
	}
	clock_.restart();
}

void GameWindow::HandleMouseMove(const sf::Event::MouseMoveEvent& ev) {
	hover_ = RoundToTile(WindowToView(window_, {ev.x, ev.y}));
}

void GameWindow::HandleMousePress(const sf::Event::MouseButtonEvent& ev) {
	auto pos = RoundToTile(WindowToView(window_, {ev.x, ev.y}));
	const auto& grid = game_.GetGrid();
	if (grid.IsEdge(pos) && grid.CanPush(pos)) {
		game_.RequestPush(pos);
	} else if (grid.IsInside(pos)) {
		game_.RequestMove(pos);
	}
}

void GameWindow::ProcessEvents() {
	sf::Event event;
	while (window_.pollEvent(event)) {
		switch (event.type) {
			case sf::Event::Closed:
				window_.close();
				break;
			case sf::Event::Resized: {
				Resize();
				break;
			}
			case sf::Event::KeyPressed:
				HandleKeypress(event.key);
				break;
			case sf::Event::MouseMoved:
				HandleMouseMove(event.mouseMove);
				break;
			case sf::Event::MouseButtonPressed:
				HandleMousePress(event.mouseButton);
				break;
			default:
				break;
		}
	}
}

void GameWindow::HandleKeypress(const sf::Event::KeyEvent& ev) {
	switch (ev.code) {
		case sf::Keyboard::Space:
			game_.RequestSkip();
			break;
		case sf::Keyboard::A:
			game_.RequestRotate(1);
			break;
		case sf::Keyboard::D:
			game_.RequestRotate(-1);
			break;
		case sf::Keyboard::I:
			game_.RequestShowMoves();
			break;
		case sf::Keyboard::P:
			{
				EagerTaxicab solver;
				game_.RequestStep(solver);
			}
			break;
		case sf::Keyboard::K:
			{
				UpwindSailer solver;
				game_.RequestStep(solver);
			}
			break;
		case sf::Keyboard::L:
			{
				SuperSolver solver;
				game_.RequestStep(solver);
			}
			break;
		case sf::Keyboard::U:
			game_.RequestUndo();
			break;
		case sf::Keyboard::X:
			game_.RequestFreePlay();
			break;
		case sf::Keyboard::B:
			game_.RequestNext(false, -1);
			break;
		case sf::Keyboard::N:
			game_.RequestNext(true, 1);
			break;
		case sf::Keyboard::M:
			game_.RequestNext(false, 1);
			break;
		case sf::Keyboard::Num3:
			game_.RequestNext(false, 3);
			break;
		default:
			break;
	}
}

void GameWindow::Run() {
	while (IsOpen()) {
		ProcessEvents();
		game_.Update();
		Draw();
		UpdateTitle();
	}
}

void GameWindow::UpdateTitle() {
	std::stringstream ss;

	ss << "Player " << game_.GetPlayer();
	ss << " |";

	int i = -1;
	for (auto score : game_.GetScores()) {
		++i;
		ss << " " << PlayerName(i) << ": " << score;
	}

	ss << " | Tick " << game_.GetTick();
	window_.setTitle(ss.str());
}
