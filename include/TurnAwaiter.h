#pragma once

#include "PlayerID.h"
#include <coroutine>

class GameScheduler;
class Game;

/**
 * @brief Custom awaiter that suspends a player's coroutine until it's their turn.
 *
 * This awaiter implements the C++20 awaitable protocol:
 * - await_ready(): Checks if we need to suspend
 * - await_suspend(): Called when suspending; registers the coroutine handle
 * - await_resume(): Called when resuming; returns void
 */
class TurnAwaiter {
public:
    /**
     * @brief Constructs an awaiter for a specific player's turn.
     * @param scheduler Reference to the game scheduler
     * @param player_id The player's ID
     * @param game Reference to the shared game state
     */
    TurnAwaiter(GameScheduler& scheduler, PlayerID player_id, Game& game);

    /**
     * @brief Checks if the coroutine needs to suspend.
     * @return False if it's not our turn (need to suspend), true otherwise.
     *
     * This is called first. If it returns true, the coroutine doesn't suspend.
     * If it returns false, await_suspend() is called.
     */
    [[nodiscard]]
    auto await_ready() const noexcept
    -> bool;

    /**
     * @brief Called when the coroutine suspends.
     * @param handle The handle of the suspending coroutine
     *
     * Registers this coroutine with the scheduler so it can be resumed
     * when it's the player's turn.
     */
    auto await_suspend(std::coroutine_handle<> handle) const noexcept
    -> void;

    /**
     * @brief Called when the coroutine resumes.
     * @return Void (no value needed).
     *
     * This is called after the scheduler resumes the coroutine.
     */
    static auto await_resume() noexcept
    -> void {}

private:
    GameScheduler& scheduler_;
    PlayerID player_id_;
    Game& game_;
};
