#pragma once
#include "Grid.h"
#include "Field.h"
#include "Point.h"
#include "Solver.h"
#include "Animation.h"
#include "InputParser.h"
#include <vector>
#include <memory>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Clock.hpp>
#include <functional>


struct ReplayState {
	std::vector<TurnInfo> turns;
	int current = 0;
};

struct GameState {
	Grid grid;
	std::vector<int> scores;
	std::vector<Field> extras;
	int player = -1;
	int tick = -1;
	int target = -1;
};

enum class State {
	kReady,
	kAnimatePush,
	kAnimateMove,

	kRequireMove,
};

enum class Mode {
	kFree,
	kPlayer,
	kReplay
};


class Game {
public:
	void InitReplay(const std::string& filename);
	void InitFreeplay(int players);
	void Update();

	Field GetExtra() const;
	State GetState() const;
	float RowDelta(int n) const;
	float ColDelta(int n) const;
	const sf::Vector2f& PlayerDelta() const;
	const std::vector<int>& GetScores() const;
	const Grid& GetGrid() const;
	int GetPlayer() const;
	int GetTick() const;
	int GetTarget() const;
	int GetColor(int x, int y) const;

	bool CanPush() const;
	bool CanMove() const;

	void RequestSkip();
	void RequestRotate(int n);
	void RequestShowMoves();
	void RequestStep(Solver& solver);
	void RequestUndo();
	void RequestFreePlay();
	void RequestNext(bool animate, int n);
	void RequestPush(const Point& edge);
	void RequestMove(const Point& move);

private:
	void ResetColors();
	void RouteColors();
	void InitInternal(Mode mode, State state);
	void SetReplay(int n);
	void SetPlayer(int player);
	void Commit();
	void NextPlayer();
	void ProcessAnimations();
	void AnimateReplay();
	void AnimatePush(Point edge, Field field, State end_state);
	void AnimateMove(Point move, State end_state);

	bool IsReachable(const Point& pos) const;
	GameState SaveState();
	void RestoreState(const GameState& gs);
	void RandomizeTargets();

	State state_;
	Grid grid_;
	std::vector<Field> extras_;
	std::vector<int> scores_;
	std::vector<std::vector<int>> targets_;

	Matrix<int> colors_;
	std::vector<float> row_delta_;
	std::vector<float> col_delta_;
	sf::Vector2f player_delta_;

	Mode mode_ = Mode::kFree;
	ReplayState replay_;			// only in replay mode
	std::vector<GameState> undo_;	// only in free mode

	int tick_ = 0;
	int player_ = 0;
	int target_ = -1;
	int player_count_ = 0;

	struct Anims {
		sf::Clock clock;
		std::unique_ptr<Animation> push;
		std::unique_ptr<Animation> move;
		std::function<void()> finish;
	} anim_;
};

