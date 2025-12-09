#pragma once

#include "Game.h"
#include "Player.h"
#include "strategies/RandomStrategy.h"
#include "strategies/CyclingStrategy.h"
#include <variant>
#include <array>

// Type alias for heterogeneous player support
// Allows mixing different strategy types in a single game
using PlayerVariant = std::variant<
    Player<RandomStrategy>,
    Player<CyclingStrategy>
    // TODO: Future strategies can be added here:
    // i.e. Player<OptimalStrategy>, Player<GreedyStrategy>
>;

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
     * @brief Runs a complete game with 4 players using mixed strategies.
     * @param players Array of 4 players with potentially different strategies
     * @return Exit code (0 for success)
     */
    static int run(std::array<PlayerVariant, 4> players);

    /**
     * @brief Runs a complete game with 4 players using the random strategy.
     */
    static void run_random();

    /**
     * @brief Benchmarks multiple games (all players with random strategy) and reports timing statistics.
     * @param num_games Number of games to run
     * @return Exit code (0 for success)
     */
    static int random_benchmark(int num_games = 1000);
};
