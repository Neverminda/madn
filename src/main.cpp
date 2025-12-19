#include <iostream>
#include <vector>
#include <chrono>
#include <print>
#include <sys/sysctl.h>

#include "Game.hpp"
#include "statistics.hpp"


int cpuCount() {
	int cpus;
	size_t size = sizeof(cpus);

	if (sysctlbyname("hw.physicalcpu", &cpus, &size, nullptr, 0) == 0) {
		return cpus;
	}
	return 1;
}

int main() {
    const int runs = 10000;
	const int cpu_count = cpuCount();
    auto start = std::chrono::steady_clock::now();

	// run games
	statistics stats = Game::runGamesInThreads(runs, cpu_count);

    auto end = std::chrono::steady_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	int colWidth = 10;
	std::println("cpuCount      {:>{}}", cpu_count, colWidth);
    std::println("time          {:>{}}", time, colWidth);
	std::println("total_games   {:>{}}", stats.games, colWidth);
    std::println("total_moves   {:>{}}", stats.moves, colWidth);
    std::println("moves_per_game{:>{}}", (double)stats.moves/runs, colWidth);
    std::println("storage_size  {:>{}} bytes", stats.moves * stats.state_size, colWidth);
}
