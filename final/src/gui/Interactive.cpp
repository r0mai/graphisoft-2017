#include "Interactive.h"
#include <iostream>
#include <SFML/Graphics.hpp>	// fixme remove



void Interactive::Init(int player) override {
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	app_.self = player;
	app_.window.create(
		sf::VideoMode(1600, 1000), "Labirintus",
		sf::Style::Default,
		settings
	);
}

void Interactive::Shutdown() override {
	std::cerr << "Final scores:" << std::endl;
	for (int i = 0, ie = scores_.size(); i < ie; ++i) {
		std::cerr << "Player " << i << ": " << scores_[i];
		std::cerr << (app_.self == i ? " *" : "") << std::endl;
	}

}

void Interactive::Update(const Grid& grid, int player) override {
	ResetGrid(grid, player);

	app_.target = -1;
	app_.state = State::kOpponent;
	app_.extra = Field(15);	// FIXME: opponent's field should be calculated

	ResetColors(app_);
	Draw(app_);
}

void Interactive::Turn(const Grid& grid, int player, int target, Field field, Callback fn) override {
	ResetGrid(grid, player);

	app_.target = target;
	app_.state = State::kPush;
	app_.extra = field;
	app_.response = {};
	callback_ = fn;
	ResetColors(app_);
}

void Interactive::Idle() override {
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

void Interactive::CheckDisplay() {
	// app_.grid.UpdatePosition(app_.self, app_.response.move);
	if (app_.target != -1 &&
		app_.response.move == app_.grid.Displays()[app_.target])
	{
		app_.grid.UpdateDisplay(app_.target, {});
		++score_;
		std::cerr << "SCORE = " << score_ << std::endl;
	}
}

void Interactive::ResetGrid(const Grid& grid, int player) {
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

void Interactive::UpdateTitle(int player) {
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

