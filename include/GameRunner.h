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
};
