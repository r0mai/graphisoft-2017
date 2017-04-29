#include "Game.h"
#include "FloodFill.h"
#include <fstream>

namespace {

class PushAnimation : public Animation {
public:
	PushAnimation(float& value, float dst_value, sf::Time max_time,
			std::function<void()> finish)
		: value_(value)
		, dst_value_(dst_value)
		, max_time_(max_time)
		, finish_(finish)
	{}

	bool Animate(const sf::Time& t) override {
		if (t > max_time_) {
			return false;
		}
		auto lambda = (t.asSeconds() / max_time_.asSeconds());
		value_ = dst_value_ * lambda;
		return true;
	}

	void Finish() override {
		finish_();
	}

private:
	float& value_;
	float dst_value_;
	sf::Time max_time_;
	std::function<void()> finish_;
};


class MoveAnimation : public Animation {
public:
	MoveAnimation(sf::Vector2f& value,
			const Game& game, const Point& dst_value,
			sf::Time max_time, std::function<void()> finish)
		: value_(value)
		, game_(game)
		, dst_value_(dst_value)
		, max_time_(max_time)
		, finish_(finish)
	{}

	bool Animate(const sf::Time& t) override {
		if (t > max_time_) {
			return false;
		}
		if (dst_value_ == Point{}) {
			value_ = sf::Vector2f{0.f, 0.f};
		} else {
			auto lambda = (t.asSeconds() / max_time_.asSeconds());
			auto src_value = game_.GetGrid().Positions()[game_.GetPlayer()];

			value_.x = lambda * (dst_value_.x - src_value.x);
			value_.y = lambda * (dst_value_.y - src_value.y);
		}
		return true;
	}

	void Finish() override {
		finish_();
	}

private:
	sf::Vector2f& value_;
	const Game& game_;
	Point dst_value_;
	sf::Time max_time_;
	std::function<void()> finish_;
};


} // namespace


void Game::InitReplay(const std::string& filename) {
	try {
		InputParser parser;
		std::ifstream input(filename);
		input.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		auto info = parser.ParseInit(parser.FromStream(input));

		TurnInfo turn;
		int player_count = 0;
		while (!(turn = parser.ParseTurn(parser.FromStream(input))).end) {
			player_count = std::max(player_count, turn.player + 1);
			replay_.turns.push_back(std::move(turn));
		}

		grid_.Init(info.width, info.height, info.displays, player_count);
		players_.resize(player_count);
		InitInternal(Mode::kReplay, State::kReady);
		SetReplay(0);
	} catch (std::ifstream::failure ex) {
		std::cerr << "error: cannot read file: " << filename << std::endl;
		exit(1);
	}
}

void Game::ResetColors() {
	colors_ = Matrix<int>(grid_.Width(), grid_.Height(), 0);
}

void Game::RouteColors() {
	auto pos = grid_.Positions()[player_];
	colors_ = FloodFill(grid_.Fields(), pos);
}

void Game::InitInternal(Mode mode, State state) {
	mode_ = mode;
	state_ = state;

	ResetColors();
	row_delta_.resize(grid_.Height(), 0.f);
	col_delta_.resize(grid_.Width(), 0.f);
	player_delta_ = {};
}

void Game::SetReplay(int n) {
	auto& info = replay_.turns[n];
	replay_.current = n;

	player_ = info.player;
	tick_ = info.tick;
	grid_ = info.grid;
	target_ = info.target;
	players_[player_].extra = info.extra;
}

Field Game::GetExtra() const {
	return players_[player_].extra;
}

State Game::GetState() const {
	return state_;
}

float Game::RowDelta(int n) const {
	return row_delta_[n];
}

float Game::ColDelta(int n) const {
	return col_delta_[n];
}

const sf::Vector2f& Game::PlayerDelta() const {
	return player_delta_;
}

int Game::GetPlayer() const {
	return player_;
}

int Game::GetTarget() const {
	return target_;
}

int Game::GetColor(int x, int y) const {
	return colors_.At(x, y);
}

const Grid& Game::GetGrid() const {
	return grid_;
}

void Game::RequestSkip() {}
void Game::RequestRotate(int n) {}

void Game::RequestShowMoves() {
	auto pos = grid_.Positions()[player_];
	colors_ = StupidFloodFill(grid_, pos, GetExtra(), false);
}

void Game::RequestStep(Solver& solver) {}
void Game::RequestFreePlay() {}

void Game::RequestUndo() {}

void Game::RequestNext(bool animate, int n) {
	if (mode_ != Mode::kReplay) {
		return;
	}

	if (state_ == State::kReady) {
		int turns = replay_.turns.size() - 1;
		auto next = std::max(0, std::min(replay_.current + n, turns));
		ResetColors();
		if (animate && next > replay_.current) {
			SetReplay(next - 1);
			AnimateReplay();
		} else {
			SetReplay(next);
		}
	}
}

void Game::Update() {
	ProcessAnimations();
}

void Game::AnimatePush(Point edge, Field field, State end_state) {
	auto dst = (edge.x == -1 || edge.y == -1 ? 1.f : -1.f);
	auto* value = (edge.x == -1 || edge.x == grid_.Width())
		? &row_delta_[edge.y]
		: &col_delta_[edge.x];

	anim_.clock.restart();
	anim_.push.reset(new PushAnimation(*value, dst, sf::milliseconds(150),
		[=]() {
			*value = 0.f;
			players_[player_].extra = grid_.Push(edge, field);
			state_ = end_state;
			RouteColors();
		}));
}

void Game::AnimateMove(Point move, State end_state) {
	anim_.clock.restart();
	anim_.move.reset(new MoveAnimation(player_delta_,
		*this, move, sf::milliseconds(150),
		[=]() {
			player_delta_ = {};
			grid_.UpdatePosition(player_, move);
			state_ = end_state;
			ResetColors();
		}));
}

void Game::ProcessAnimations() {
	auto elapsed = anim_.clock.getElapsedTime();
	auto& push = anim_.push;
	auto& move = anim_.move;

	if (push) {
		if (!push->Animate(elapsed)) {
			push->Finish();
			push.reset();
			anim_.clock.restart();
		}
		return;
	}
	if (move) {
		if (!move->Animate(elapsed)) {
			move->Finish();
			move.reset();
			anim_.clock.restart();
		}
		return;
	}
	if (anim_.finish) {
		anim_.finish();
		anim_.finish = {};
	}
}

void Game::AnimateReplay() {
	auto current = replay_.current;
	auto next = current + 1;

	const auto& src = replay_.turns[current];
	const auto& dst = replay_.turns[next];
	auto delta = dst.grid.Diff(src.grid, src.extra, src.player);

	state_ = State::kAnimatePush;
	AnimatePush(delta.edge, delta.extra, State::kAnimateMove);
	AnimateMove(delta.move, State::kReady);
	anim_.finish = [=]() {
		SetReplay(next);
	};
}
