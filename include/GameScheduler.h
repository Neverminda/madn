#pragma once

#include "PlayerID.h"
#include <coroutine>
#include <array>

// Forward declarations
class Game;
class TurnAwaiter;

/**
 * @brief Coordinates the execution of player coroutines in a turn-based manner.
 *
 * The GameScheduler manages coroutine handles for all 4 players and ensures
 * that only the current player's coroutine runs. When a player completes their
 * turn, the scheduler resumes the next player's coroutine.
 *
 * This replaces the mutex + condition variable synchronization used in the
 * threading model with cooperative coroutine scheduling.
 */
class GameScheduler final {
public:
    /**
     * @brief Constructs a scheduler for the given game.
     * @param game Reference to the shared game state
     */
    explicit GameScheduler(Game& game);

    /**
     * @brief Registers a player's coroutine handle when they wait for their turn.
     * @param player_id The player's ID
     * @param handle The coroutine handle to resume when it's their turn
     */
    auto register_waiting_player(PlayerID player_id, std::coroutine_handle<> handle)
    -> void;

    /**
     * @brief Resumes the coroutine for the current player.
     *
     * If the game is over, resumes all waiting coroutines so they can exit.
     * Otherwise, resumes only the coroutine for game.current_player.
     */
    auto resume_current_player()
    -> void;

    /**
     * @brief Called by a player coroutine after completing their turn.
     *
     * This triggers the scheduler to resume the next player's coroutine.
     */
    auto notify_turn_complete()
    -> void;

    /**
     * @brief Runs the game loop until completion.
     *
     * Starts by resuming Player A's coroutine. The game then proceeds
     * as coroutines suspend/resume in turn order until game over.
     */
    auto run_until_complete()
    -> void;

    /**
     * @brief Creates a TurnAwaiter for the specified player.
     * @param player_id The player's ID
     * @return TurnAwaiter that will suspend until it's the player's turn
     */
    [[nodiscard]]
    auto wait_for_turn(PlayerID player_id)
    -> TurnAwaiter;

private:
    Game& game_;
    std::array<std::coroutine_handle<>, 4> waiting_handles_;
};
