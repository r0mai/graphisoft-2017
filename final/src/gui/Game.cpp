#include "Game.h"
#include "FloodFill.h"
#include "Util.h"
#include <fstream>
#include <algorithm>
#include <numeric>

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
		scores_.resize(player_count, 0);
		extras_.resize(player_count, Field(15));
		player_count_ = player_count;

		InitInternal(Mode::kReplay, State::kReady);
		SetReplay(0);
	} catch (std::ifstream::failure ex) {
		std::cerr << "error: cannot read file: " << filename << std::endl;
		exit(1);
	}
}

void Game::RandomizeTargets() {
	auto displays = grid_.Displays().size();
	targets_.resize(player_count_);
	for (int i = 0; i < player_count_; ++i) {
		auto& vec = targets_[i];
		vec.resize(displays);
		std::iota(vec.begin(), vec.end(), 0);
		std::random_shuffle(vec.begin(), vec.end());
	}
}

void Game::InitFreeplay(int player_count) {
	srand(10372);

	int width = 14;
	int height = 8;
	int max_d = width * height / 4;
	int displays = std::min(20, max_d);

	grid_.Init(width, height, displays, player_count);
	grid_.Randomize();
	grid_.RandomizeBlocked(20); // will be restricted anyway

	scores_.resize(player_count, 0);
	extras_.resize(player_count, Field(15));
	player_count_ = player_count;

	RandomizeTargets();
	InitInternal(Mode::kFree, State::kReady);
	SetPlayer(0);
	undo_.push_back(SaveState());
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

void Game::SetPlayer(int player) {
	player_ = player;
	target_ = -1;

	for (auto x : targets_[player]) {
		if (IsValid(grid_.Displays()[x])) {
			target_ = x;
			break;
		}
	}
}

void Game::SetReplay(int n) {
	auto& info = replay_.turns[n];
	replay_.current = n;

	player_ = info.player;
	tick_ = info.tick;
	grid_ = info.grid;
	target_ = info.target;
	extras_[player_] = info.extra;
	scores_ = info.scores;
}

Field Game::GetExtra() const {
	return extras_[player_];
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

int Game::GetTick() const {
	return tick_;
}

int Game::GetTarget() const {
	return target_;
}

int Game::GetColor(int x, int y) const {
	return colors_.At(x, y);
}

const std::vector<int>& Game::GetScores() const {
	return scores_;
}

const Grid& Game::GetGrid() const {
	return grid_;
}

void Game::RequestSkip() {
	if (CanMove()) {
		state_ = State::kReady;
		Commit();
		ResetColors();
	}
}

void Game::RequestRotate(int n) {
	auto& field = extras_[player_];
	if (n >= 0) {
		while (n-- > 0) {
			field = RotateLeft(field);
		}
	} else {
		while (n++ < 0) {
			field = RotateRight(field);
		}
	}
}

void Game::RequestShowMoves() {
	auto pos = grid_.Positions()[player_];
	colors_ = StupidFloodFill(grid_, pos, GetExtra(), false);
}

void Game::RequestStep(Solver& solver) {
	if (mode_ != Mode::kFree || state_ != State::kReady || target_ < 0) {
		return;
	}

	solver.Init(player_);
	auto result = solver.SyncTurn(grid_, player_, target_, extras_[player_]);

	if (result.push.edge == Point{}) {
		std::cerr << "Invalid push" << std::endl;
		return;
	}

	state_ = State::kAnimatePush;
	AnimatePush(result.push.edge, result.push.field, State::kAnimateMove);
	AnimateMove(result.move, State::kReady);
	anim_.finish = [=]() {
		Commit();
	};
}

void Game::RequestFreePlay() {
	if (mode_ == Mode::kReplay && state_ == State::kReady) {
		mode_ = Mode::kFree;
		undo_.push_back(SaveState());

		RandomizeTargets();
		if (target_ >= 0) {
			auto& vec = targets_[player_];
			vec.insert(vec.begin(), target_);
		}
		SetPlayer(player_);
	}
}

void Game::RequestUndo() {
	if (mode_ == Mode::kFree && state_ == State::kReady && undo_.size() > 1) {
		undo_.pop_back();
		RestoreState(undo_.back());
	}
}

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
			auto new_extra = grid_.Push(edge, field);
			extras_[player_] = new_extra;
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
			if (IsValid(move)) {
				grid_.UpdatePosition(player_, move);
			}
			state_ = end_state;
			ResetColors();
		}));
}

void Game::ProcessAnimations() {
	auto& timer = anim_.clock;
	auto& push = anim_.push;
	auto& move = anim_.move;

	if (push) {
		if (push->Animate(timer.getElapsedTime())) {
			return;
		}
		push->Finish();
		push.reset();
		timer.restart();
	}
	if (move) {
		if (move->Animate(timer.getElapsedTime())) {
			return;
		}
		move->Finish();
		move.reset();
		timer.restart();
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

bool Game::CanPush() const {
	return mode_ == Mode::kFree && state_ == State::kReady;
}

bool Game::CanMove() const {
	return state_ == State::kRequireMove;
}

void Game::RequestPush(const Point& edge) {
	if (CanPush() && grid_.IsEdge(edge)) {
		state_ = State::kAnimatePush;
		AnimatePush(edge, extras_[player_], State::kRequireMove);
	}
}

void Game::RequestMove(const Point& move) {
	if (CanMove() && grid_.IsInside(move) && IsReachable(move)) {
		state_ = State::kAnimateMove;
		AnimateMove(move, State::kReady);
		anim_.finish = [=]() {
			Commit();
		};
	}
}

bool Game::IsReachable(const Point& pos) const {
	auto colors = FloodFill(grid_.Fields(), grid_.Positions()[player_]);
	return colors.At(pos);
}

void Game::Commit() {
	if (target_ >= 0) {
		auto player_pos = grid_.Positions()[player_];
		auto display_pos = grid_.Displays()[target_];
		if (player_pos == display_pos) {
			grid_.UpdateDisplay(target_, {});
			scores_[player_] += 1;
		}
	}

	NextPlayer();
	undo_.push_back(SaveState());
}

void Game::NextPlayer() {
	auto next = player_ + 1;
	if (next == player_count_) {
		next = 0;
		tick_ += 1;
	}
	SetPlayer(next);
}

GameState Game::SaveState() {
	GameState gs;
	gs.grid = grid_;
	gs.scores = scores_;
	gs.extras = extras_;
	gs.player = player_;
	gs.tick = tick_;
	gs.target = target_;
	return gs;
}

void Game::RestoreState(const GameState& gs) {
	grid_ = gs.grid;
	scores_ = gs.scores;
	extras_ = gs.extras;
	player_ = gs.player;
	tick_ = gs.tick;
	target_ = gs.target;
}
