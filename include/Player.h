#pragma once

#include "PlayerID.h"
#include "PlayerStrategy.h"
#include "Task.h"
#include "GameScheduler.h"
#include "TurnAwaiter.h"
#include <print>

class Game;

/**
 * @brief Represents a player in the game.
 *
 * Template class using compile-time polymorphism via concepts.
 * Manages player state and coordinates with a strategy for decision-making.
 * Each player runs in its own thread and takes turns based on the game state.
 *
 * Player accepts any PlayerStrategy - individual strategies can enforce
 * CompactStrategy if they want to guarantee memory efficiency.
 */
template<PlayerStrategy Strategy>
class Player final {
public:
    /**
     * @brief Constructs a player with a given strategy.
     * @param id Player ID
     * @param strat Strategy for move selection (moved into player)
     */
    Player(const PlayerID id, Strategy strat)
        : player_id(id)
        , strategy(std::move(strat)) {}

    /**
     * @brief Main game loop for this player (runs as a coroutine).
     * @param game Reference to the shared game state
     * @param scheduler Reference to the coroutine scheduler
     */
    auto play_game(Game &game, GameScheduler &scheduler)
    -> Task
    {
        while (true) {
            co_await scheduler.wait_for_turn(player_id); // this creates a TurnAwaiter

            if (game.is_game_over) break;

            // roll
            const int roll = game.roll_dice();
            const bool earned_another_turn = (roll == 6);

            // update game state based on strategy
            strategy.make_move(game, player_id, roll);
            game.print_game_state(roll);
            check_for_win(game);
            if (!earned_another_turn || game.is_game_over) game.current_player = next_player(game.current_player);

            scheduler.notify_turn_complete();
        }

        co_return;
    }

    auto check_for_win(Game &game)
    -> void
    {
        if (game.check_for_win(player_id)) {
            game.winner = player_id;
            game.is_game_over = true;
        }
    }

    [[nodiscard]]
    auto get_id() const
    -> PlayerID { return player_id; }

private:
    PlayerID player_id;
    Strategy strategy;
};
