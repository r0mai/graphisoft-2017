#pragma once
#include "Grid.h"
#include "Point.h"
#include "Field.h"
#include <functional>
#include <condition_variable>
#include <mutex>


struct Response {
	struct {
		Point edge;
		Field field;
	} push;

	Point move;
};


class Solver {
public:
	using Callback = std::function<void(const Response&)>;

	virtual ~Solver() {}
	virtual void Init(int player) = 0;
	virtual void Shutdown() = 0;
	virtual void Update(const Grid& grid, int player) = 0;
	virtual void Turn(const Grid& grid, int player, int target, Field field, Callback fn) = 0;
	virtual void Idle() = 0;

	Response SyncTurn(const Grid& grid, int player, int target, Field field) {
		Response result;
		std::mutex m;
		std::condition_variable cv;
		bool finished = false;

		Turn(grid, player, target, field, [&](const Response& response) {
			// This may be called in a different thread;
			// otherwise it must be called before Turn() returns.
			std::unique_lock<std::mutex> lk(m);
			finished = true;
			result = response;
			lk.unlock();
			cv.notify_all();
		});

		std::unique_lock<std::mutex> lk(m);
		cv.wait(lk, [&]{ return finished; });
		return result;
	}
};
