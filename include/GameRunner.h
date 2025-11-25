#pragma once

class Game;

/**
 * @brief Orchestrates game execution with multiple player threads.
 *
 * Responsible for:
 * - Creating and managing player threads
 * - Coordinating game start and completion
 * - User interaction (prompts, results)
 */
class GameRunner final {
public:
    /**
     * @brief Runs a complete game with 4 players using the random strategy.
     * @return Exit code (0 for success)
     */
    static int run_random();

    /**
     * @brief Benchmarks multiple games (all players with random strategy) and reports timing statistics.
     * @param num_games Number of games to run
     * @return Exit code (0 for success)
     */
    static int random_benchmark(int num_games = 1000);
};
