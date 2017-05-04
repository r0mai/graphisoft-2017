#include "Solver.h"
#include <condition_variable>
#include <mutex>

Response Solver::SyncTurn(const Grid& grid, int player, int target, Field field,
		int nextTarget) {
	Response result;
	std::mutex m;
	std::condition_variable cv;
	bool finished = false;

	Turn(grid, player, target, field, nextTarget, [&](const Response& response)
	{
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
