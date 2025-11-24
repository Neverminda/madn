#pragma once

#include "PlayerID.h"
#include "PlayerStrategy.h"
#include "Dice.h"
#include <random>
#include <print>

class Game;

/**
 * @brief Represents a player in the game.
 *
 * Template class using compile-time polymorphism via concepts.
 * Manages player state and coordinates with a strategy for decision-making.
 * Each player runs in its own thread and takes turns based on game state.
 *
 * Template parameters enable both strategy pattern and testable dice injection.
 * Player accepts any PlayerStrategy - individual strategies can enforce
 * CompactStrategy if they want to guarantee memory efficiency.
 */
template<PlayerStrategy Strategy, DiceRollable Dice = StandardDice>
class Player final {
public:
    /**
     * @brief Constructs a player with a given strategy.
     * @param id Player ID
     * @param strat Strategy for move selection (moved into player)
     */
    Player(const PlayerID id, Strategy strat)
        : player_id(id)
        , strategy(std::move(strat))
        , dice_rng(std::random_device{}() + to_int(id))
        , dice(dice_rng) {}

    /**
     * @brief Main game loop for this player (runs in a thread).
     * @param game Reference to the shared game state
     */
    auto play_game(Game &game)
    -> void {
        const char p_char = to_char(player_id);

        while (true) {
            std::unique_lock lock(game.mtx);

            // Wait until it's our turn OR the game is over
            game.cv.wait(lock, [&]() -> bool {
                return game.current_player == player_id || game.is_game_over;
            });

            // If the game is over, terminate the thread
            if (game.is_game_over) break;

            // --- It's our turn ---
            const int roll = dice();
            const bool earned_another_turn = (roll == 6);

            // Delegate the move to the strategy (no virtual call!)
            strategy.make_move(game, player_id, roll);
            game.print_game_state(roll);

            // Check for win
            if (game.check_for_win(player_id)) {
                game.is_game_over = true;
                std::println("\n*** PLAYER {} HAS WON! ***", p_char);
            }

            // --- Pass turn ---
            if (!earned_another_turn || game.is_game_over)
                game.current_player = next_player(game.current_player);

            lock.unlock();
            game.cv.notify_all();
        }
    }

    [[nodiscard]]
    auto get_id() const
    -> PlayerID { return player_id; }

private:
    PlayerID player_id;
    Strategy strategy;
    std::mt19937 dice_rng;
    Dice dice;              // Mockable dice for testing! // TODO: add tests
};
