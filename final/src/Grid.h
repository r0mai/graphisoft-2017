#pragma once
#include <vector>
#include <set>
#include <cstdlib>
#include <cassert>
#include <iostream>

#include "Point.h"
#include "Matrix.h"
#include "Field.h"

class Grid {
public:
	int Width() const;
	int Height() const;
	int PlayerCount() const;
	int DisplayCount() const;
	int ActiveDisplayCount() const;
	Point Size() const;
	const Matrix<Field>& Fields() const;
	const std::vector<Point>& Displays() const;
	const std::vector<Point>& Positions() const;
	bool IsNeighbor(int player, int display) const;
	bool IsNeighbor(const Point& p, const Point& q) const;
	bool IsEdge(const Point& pos) const;
	bool IsInside(const Point& pos) const;
	bool CanPush(const Point& pos) const;

	void Init(int width, int height, int displays, int players);
	void Randomize();
	void RandomizeBlocked(int n);
	void ResetDisplays();
	void UpdateFields(std::vector<Field> fields);
	void UpdateDisplay(int index, const Point& pos);
	void UpdatePosition(int player, const Point& pos);
	void AddBlocked(int x, int y);
	bool IsBlockedX(int x) const;
	bool IsBlockedY(int y) const;

	Field At(int x, int y) const;
	Field At(const Point& pos) const;
	Field Push(const Point& pos, Field t);
	Field Push(int c, int p, int k, Field t);

	struct Delta {
		Point edge;
		Field extra = Field(0);
		Point move;
		bool scored = false;
	};

	// @return 	the move that led from {grid, extra} to this.
	Delta Diff(const Grid& grid, Field extra, int player) const;

	// @return	the tile that is missing after extra was pushed to grid
	Field TileDiff(const Grid& grid, Field extra) const;

	// @return 	true if a single display got deactivated compared to the other
	bool ScoreDiff(const Grid& grid) const;

private:
	std::vector<Point> displays_;
	std::vector<Point> positions_;
	std::vector<Point> blocked_;
	std::set<int> blocked_cols_;
	std::set<int> blocked_rows_;
	Matrix<Field> fields_;
};

std::ostream& operator<<(std::ostream& os, const Grid& grid);
