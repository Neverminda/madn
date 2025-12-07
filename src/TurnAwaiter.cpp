#include "TurnAwaiter.h"
#include "GameScheduler.h"
#include "Game.h"

TurnAwaiter::TurnAwaiter(GameScheduler& scheduler, const PlayerID player_id, Game& game)
    : scheduler_(scheduler)
    , player_id_(player_id)
    , game_(game) {}

auto TurnAwaiter::await_ready() const noexcept
-> bool
{
    // Don't suspend if it's our turn or the game is over
    return game_.current_player == player_id_ || game_.is_game_over;
}

auto TurnAwaiter::await_suspend(const std::coroutine_handle<> handle) const noexcept
-> void
{
    // Register this coroutine with the scheduler
    scheduler_.register_waiting_player(player_id_, handle);
}
