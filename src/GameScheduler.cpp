#include "GameScheduler.h"
#include "TurnAwaiter.h"
#include "Game.h"

GameScheduler::GameScheduler(Game& game)
    : game_(game) {
    waiting_handles_.fill(nullptr);
}

auto GameScheduler::register_waiting_player(const PlayerID player_id, const std::coroutine_handle<> handle)
-> void { waiting_handles_[to_int(player_id)] = handle; }

auto GameScheduler::resume_current_player()
-> void
{
    if (game_.is_game_over) {
        // Game is over - resume all waiting coroutines so they can exit
        for (auto& handle : waiting_handles_) {
            if (handle && !handle.done()) handle.resume();
        }
        return;
    }

    // Resume only the current player's coroutine
    const int current_idx = to_int(game_.current_player);
    if (const auto handle = waiting_handles_[current_idx]; handle && !handle.done()) {
        waiting_handles_[current_idx] = nullptr; // Clear before resuming
        handle.resume();
    }
}

auto GameScheduler::notify_turn_complete()
-> void { resume_current_player(); }

auto GameScheduler::run_until_complete()
-> void
{
    // Start the game by resuming Player A's coroutine
    // This will trigger the chain of coroutine suspensions/resumptions until the game is over.
    resume_current_player();
}

auto GameScheduler::wait_for_turn(const PlayerID player_id)
-> TurnAwaiter { return TurnAwaiter{*this, player_id, game_}; }
