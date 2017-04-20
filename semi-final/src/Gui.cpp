#include "Point.h"
#include "Grid.h"
#include "Util.h"
#include "FloodFill.h"
#include "EagerTaxicab.h"
#include "Solver.h"
#include "Client.h"
#include "Hsv2rgb.h"

#include <cassert>
#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <cstdlib>
#include <SFML/Graphics.hpp>
#include <boost/program_options.hpp>
#include <chrono>


using Clock = std::chrono::steady_clock;
using Duration = std::chrono::duration<double>;


struct Action {
	int player = -1;
	int display = -1;
	Point push;
	Point move;
	Field field;
};

struct Game {
	Grid grid;
	std::vector<Field> extras;
	std::vector<int> scores;
	std::vector<Action> undo_stack;
	int tick = -1;
	int player = -1;

	int players = 0;
	int displays = 0;
	int remain = 0;
};

enum class State {
	kOpponent,
	kPush,
	kMove,
	kDone,
	kGameOver,
	kAnimatePush,
	kAnimateMove,
	kUndo
};

class Animation {
public:
	virtual ~Animation() {}
	virtual bool Animate(Duration delta_time) = 0;
};

struct App {
	sf::RenderWindow window;
	Matrix<int> colors;
	Point hover;
	bool invalid = true;

	State state;

	Grid grid;
	Field extra;
	int self = 0;
	int target = 0;

	std::vector<float> row_delta;
	std::vector<float> col_delta;
	sf::Vector2f player_delta;

	struct Anims {
		Clock::time_point start_t;
		std::unique_ptr<Animation> push;
		std::unique_ptr<Animation> move;
	} anim;

	Response response;
};

class AnimatePush : public Animation {
public:
	AnimatePush(App& app, const Point& edge, Field extra, Duration duration)
		: app_(app)
		, duration_(duration)
		, extra_(extra)
		, edge_(edge)
	{
		dest_ = (edge.x == -1 || edge.y == -1 ? 1.f : -1.f);
		if (edge.x == -1 || edge.x == app.grid.Width()) {
			var_ = &app.row_delta[edge.y];
		} else {
			var_ = &app.col_delta[edge.x];
		}
	}

	bool Animate(Duration delta_time) override {
		if (delta_time >= duration_) {
			*var_ = 0.f;
			app_.extra = app_.grid.Push(edge_, extra_);
			return false;
		}
		*var_ = dest_ * delta_time / duration_;
		return true;
	}

private:
	App& app_;
	Field extra_;
	Point edge_;
	Duration duration_;
	float* var_ = nullptr;
	float dest_ = 0;
};

class AnimateMove : public Animation {
public:
	AnimateMove(App& app, Point move, Duration duration)
		: app_(app)
		, duration_(duration)
		, move_(move)
	{}

	bool Animate(Duration delta_time) override {
		if (!is_initialized_) {
			auto current = app_.grid.Positions()[app_.self];
			delta_ = sf::Vector2f(move_.x - current.x, move_.y - current.y);
			is_initialized_ = true;
		}

		if (delta_time >= duration_) {
			app_.player_delta = {};
			app_.grid.UpdatePosition(app_.self, move_);
			return false;
		}
		auto ratio = delta_time / duration_;
		app_.player_delta.x = delta_.x * ratio;
		app_.player_delta.y = delta_.y * ratio;
		return true;
	}

private:
	App& app_;
	Point move_;
	sf::Vector2f delta_;
	Duration duration_;
	bool is_initialized_ = false;
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
			if (app.state == State::kPush) {
				EagerTaxicab solver;
				solver.Init(app.self);
				auto rp = solver.SyncTurn(
					app.grid, app.self, app.target, app.extra);

				app.response = rp;
				app.anim.start_t = Clock::now();
				app.anim.push = std::make_unique<AnimatePush>(
					app, rp.push.edge, rp.push.field, Duration(0.15));
				app.anim.move = std::make_unique<AnimateMove>(
					app, rp.move, Duration(0.25));
				app.state = State::kAnimatePush;
			}
		case sf::Keyboard::U:
			if (app.state == State::kPush) {
				app.state = State::kUndo;
			}
			break;

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
#if 1
	app.colors = FloodFill(app.grid.Fields(), pos);
#else
	app.colors = FullFloodFill(app.grid.Fields(), 2);
	FloodFillTo(app.colors, app.grid.Fields(), pos, 1);
#endif
}

void ProcessAnimations(App& app) {
	auto current_t = Clock::now();
	if (app.state == State::kAnimatePush) {
		if (!app.anim.push->Animate(current_t - app.anim.start_t)) {
			app.state = app.anim.move ? State::kAnimateMove : State::kMove;
			app.anim.start_t = current_t; // for next animation
			app.anim.push.reset();
			UpdateColors(app);
		}
	} else if (app.state == State::kAnimateMove) {
		if (!app.anim.move->Animate(current_t - app.anim.start_t)) {
			app.state = State::kDone;
			app.anim.start_t = current_t; // for next animation
			app.anim.move.reset();
			ResetColors(app);
		}
	}
}

void HandleMouseMoved(App& app, const sf::Event::MouseMoveEvent& ev) {
	auto pos = RoundToTile(WindowToView(app, {ev.x, ev.y}));
	app.hover = pos;
}

void HandleMousePressed(App& app, const sf::Event::MouseButtonEvent& ev) {
	auto pos = RoundToTile(WindowToView(app, {ev.x, ev.y}));
	if (app.state == State::kPush && IsEdge(app, pos)) {
		app.response.push.edge = pos;
		app.response.push.field = app.extra;
		app.state = State::kAnimatePush;
		app.anim.push = std::make_unique<AnimatePush>(app, pos, app.extra, Duration(0.15));
		app.anim.start_t = Clock::now();
	} else if (app.state == State::kMove && IsInside(app, pos) &&
		app.colors.At(pos) == 1)
	{
		app.response.move = pos;
		app.state = State::kAnimateMove;
		app.anim.move = std::make_unique<AnimateMove>(app, pos, Duration(0.25));
		app.anim.start_t = Clock::now();
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

	sf::Color color;
	if (color_id == 0) {
		color = sf::Color(0xf0, 0xf0, 0xe0);
	} else if (color_id == 1) {
		color = sf::Color(0xc4, 0xf9, 0x4f);
	} else if (color_id != 0) {
		color = HSVtoRGB(std::fmod(color_id * 11.3456, 60), 0.45, 1);
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
		if (!IsValid(pos)) {
			continue;
		}

		bool self = !!(p.second & (1<<app.self));
		sf::Vector2f delta{app.row_delta[pos.y], app.col_delta[pos.x]};

		int k = -1;
		for (int i = 0; i < 4; ++i) {
			k += !!(p.second & (1<<i));
		}

		for (int i = 0; i < 4; ++i) {
			if (!!(p.second & (1<<i)) && i != app.self) {
				auto color = colors[i];
				auto dot = CreateDiamond(
					sf::Vector2f(pos.x, pos.y) + delta + float(k) * offset);

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
			auto dp = sf::Vector2f(pos.x, pos.y) + delta + app.player_delta;
			auto dot = CreateDiamond(dp);
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
			sf::Vector2f delta{app.row_delta[pos.y], app.col_delta[pos.x]};
			auto dot = CreateSquare(sf::Vector2f(pos.x, pos.y) + delta);
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
	if (app.invalid) {
		app.invalid = false;
		AdjustView(app);
	}

	auto& window = app.window;
	const auto& hover = app.hover;
	auto size = app.grid.Size();
	auto is_push = app.state == State::kPush;

	window.clear(sf::Color(0x33, 0x33, 0x33));

	for (int y = 0; y < size.y; ++y) {
		for (int x = 0; x < size.x; ++x) {
			auto color_id = app.colors.At(x, y);
			sf::Vector2f pos(app.row_delta[y] + x, app.col_delta[x] + y);
			DrawTile(app, pos, app.grid.At(x, y), color_id);
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

void ResetAppState(const Game& game, App& app) {
	app.grid = game.grid;
	app.self = game.player;
	app.target = NextDisplay(app.grid);
	app.state = State::kPush;
	app.extra = game.extras[game.player];
	app.response.push.edge = {};
	app.response.move = {};
	ResetColors(app);
}

void NextPlayer(Game& game, App& app) {
	game.player = (game.player + 1) % game.players;
	if (game.player == 0) {
		++game.tick;
	}
	ResetAppState(game, app);
}

void UndoMove(Game& game, App& app) {
	if (game.undo_stack.empty()) {
		app.state = State::kPush;
		return;
	}

	auto undo = game.undo_stack.back();
	game.undo_stack.pop_back();

	if (undo.display >= 0) {
		auto pos = game.grid.Positions()[undo.player];
		game.grid.UpdateDisplay(undo.display, pos);
		++game.remain;
		--game.scores[undo.player];
	}
	if (IsValid(undo.move)) {
		game.grid.UpdatePosition(undo.player, undo.move);
	}
	game.extras[undo.player] = game.grid.Push(undo.push, undo.field);

	if (game.player == 0) {
		--game.tick;
	}
	game.player = undo.player;
	ResetAppState(game, app);
}

Point ReversePush(const Grid& grid, const Point& pos) {
	Point rev = pos;
	if (rev.x == -1) {
		rev.x = grid.Width();
	} else if (rev.x == grid.Width()) {
		rev.x = -1;
	} else if (rev.y == -1) {
		rev.y = grid.Height();
	} else if (rev.y == grid.Height()) {
		rev.y = -1;
	}
	return rev;
}

void ApplyMove(Game& game, App& app) {
	auto& extra = game.extras[game.player];
	auto& push = app.response.push;
	extra = game.grid.Push(push.edge, push.field);

	Action undo;
	undo.player = game.player;
	undo.field = extra;
	undo.push = ReversePush(game.grid, push.edge);

	if (IsValid(app.response.move)) {
		// TODO: check if this is a valid move
		undo.move = game.grid.Positions()[game.player];

		auto target = app.target;
		game.grid.UpdatePosition(game.player, app.response.move);
		if (target != -1 &&
			app.response.move == game.grid.Displays()[app.target])
		{
			undo.display = app.target;
			--game.remain;
			++game.scores[game.player];
			game.grid.UpdateDisplay(target, {});
			std::cerr << "SCORES:";
			for (auto x : game.scores) {
				std::cerr << " " << x;
			}
			std::cerr << std::endl;
		}
	}

	game.undo_stack.push_back(undo);
}

void Run(Game& game, App& app) {
	while (app.window.isOpen()) {
		HandleEvents(app);
		ProcessAnimations(app);
		Draw(app);

		if (app.state == State::kUndo) {
			UndoMove(game, app);
		}

		if (app.state == State::kDone) {
			ApplyMove(game, app);
			NextPlayer(game, app);
			if (game.remain == 0) {
				app.state = State::kGameOver;
			}
		}
	}
}

void InitGame(Game& game, int players) {
	int w = 14, h = 8;
	int max_d = w * h / 4;

	game.displays = std::min(20, max_d);
	game.players = players;
	game.grid.Init(w, h, game.displays, game.players);
	game.grid.Randomize();
	game.extras.resize(game.players, Field(15));
	game.scores.resize(players);
	game.remain = game.displays;
}

void InitApp(const Game& game, App& app) {
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	app.window.create(
		sf::VideoMode(1600, 1000), "Labirintus",
		sf::Style::Default,
		settings
	);

	app.grid = game.grid;
	app.row_delta.resize(app.grid.Height());
	app.col_delta.resize(app.grid.Width());

	AdjustView(app);
}

void HotSeat(int players) {
	Game game;
	App app;

	srand(10371);
	InitGame(game, players);
	InitApp(game, app);
	NextPlayer(game, app);
	Run(game, app);
}


class InteractiveSolver : public Solver {
public:
	void Init(int player) override {
		sf::ContextSettings settings;
		settings.antialiasingLevel = 8;

		app_.self = player;
		app_.window.create(
			sf::VideoMode(1600, 1000), "Labirintus",
			sf::Style::Default,
			settings
		);
	}

	void Shutdown() override {
		std::cerr << "Final scores:" << std::endl;
		for (int i = 0, ie = scores_.size(); i < ie; ++i) {
			std::cerr << "Player " << i << ": " << scores_[i];
			std::cerr << (app_.self == i ? " *" : "") << std::endl;
		}

	}

	void Update(const Grid& grid, int player) override {
		ResetGrid(grid, player);

		app_.target = -1;
		app_.state = State::kOpponent;
		app_.extra = Field(15);	// FIXME: opponent's field should be calculated

		ResetColors(app_);
		Draw(app_);
	}

	void Turn(const Grid& grid, int player, int target, Field field, Callback fn) override {
		ResetGrid(grid, player);

		app_.target = target;
		app_.state = State::kPush;
		app_.extra = field;
		app_.response = {};
		callback_ = fn;
		ResetColors(app_);
	}

	void Idle() override {
		if (app_.window.isOpen()) {
			if (!has_grid_) {
				return;
			}
			HandleEvents(app_);
			ProcessAnimations(app_);

			if (app_.state == State::kDone && callback_) {
				// CheckDisplay();

				app_.state = State::kOpponent;
				ResetColors(app_);
				callback_(app_.response);
				callback_ = {};
			}

			Draw(app_);
		} else {
			exit(0);
		}
	}

private:
	void CheckDisplay() {
		// app_.grid.UpdatePosition(app_.self, app_.response.move);
		if (app_.target != -1 &&
			app_.response.move == app_.grid.Displays()[app_.target])
		{
			app_.grid.UpdateDisplay(app_.target, {});
			++score_;
			std::cerr << "SCORE = " << score_ << std::endl;
		}
	}

	void ResetGrid(const Grid& grid, int player) {
		app_.grid = grid;
		if (!has_grid_) {
			has_grid_ = true;
			app_.row_delta.resize(grid.Height());
			app_.col_delta.resize(grid.Width());
			scores_.resize(grid.PlayerCount());
			displays_ = grid.ActiveDisplayCount();
		}

		auto displays = grid.ActiveDisplayCount();
		if (displays < displays_) {
			assert(displays == displays_ - 1);
			displays_ = displays;
			int player_count = grid.PlayerCount();
			scores_[(player - 1 + player_count) % player_count] += 1;
		}

		UpdateTitle(player);
	}

	void UpdateTitle(int player) {
		std::stringstream ss;

		ss << "Player " << app_.self << " | Scores:";
		for (int i = 0, ie = scores_.size(); i < ie; ++i) {
			ss << " " << scores_[i];
		}
		ss << " | ";
		if (player == app_.self) {
			ss << " Your turn";
		} else {
			ss << " Waiting for Player " << player;
		}

		app_.window.setTitle(ss.str());
	}

	int score_ = 0;
	bool has_grid_ = false;
	int displays_ = 0;
	std::vector<int> scores_;
	App app_;
	Callback callback_;
};


int main(int argc, char* argv[]) {
	namespace po = boost::program_options;
	po::options_description desc{"Allowed Options"};
	desc.add_options()
		("help,h", "this help message")
		("hotseat,s", po::value<int>()->value_name("N"), "hotseat mode with N players")
		("host,H", po::value<std::string>(), "hostname to connect, defaults to localhost")
		("team,t", po::value<std::string>(), "teamname to use during login")
		("password,p", po::value<std::string>(), "password to use for authentication");
	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);
	} catch(std::exception& e) {
		std::cerr << "error: " << e.what() << std::endl;
		return 1;
	}

	bool is_hotseat = vm.count("hotseat");

	int port = 42500;
	std::string host_name = "localhost";
	std::string team_name = "the_hypnotoad";
	std::string password = "******";

	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return 0;
	}

	if (vm.count("host")) {
		host_name = vm["host"].as<std::string>();
	}

	if (vm.count("team")) {
		team_name = vm["team"].as<std::string>();
	}

	if (vm.count("password")) {
		password = vm["password"].as<std::string>();
	}

	// 0 means here: server choose randomly 1 to 10
	int task_id = argc > 1 ? std::atoi(argv[1]) : 0;


	if (is_hotseat) {
		int players = vm["hotseat"].as<int>();
		HotSeat(players);
	} else {
		try {
			platform_dep::enable_socket _;
			InteractiveSolver solver;
			Client(host_name, port, team_name, password, task_id).Run(solver);
		} catch(std::exception& e) {
			std::cerr << "Exception thrown. what(): " << e.what() << std::endl;
		}
	}

	return 0;
}
