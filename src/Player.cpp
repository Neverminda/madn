#include "Player.h"
#include "Game.h"
#include "PlayerStrategy.h"
#include <print>

Player::Player(const PlayerID id, std::unique_ptr<PlayerStrategy> strat)
    : player_id(id)
    , strategy(std::move(strat))
    , dice_dist(1, 6)
    , dice_rng(std::random_device{}() + to_int(id)) {}

void Player::play_game(Game& game) {
    const char p_char = to_char(player_id);

    while (true) {
        // Acquire lock to access shared data
        std::unique_lock lock(game.mtx);

        // Wait until it's our turn OR the game is over
        game.cv.wait(lock, [&]() -> bool {
            return game.current_player == player_id || game.game_over;
        });

        // If the game is over, terminate the thread
        if (game.game_over) break;

        // --- It's our turn ---
        const int roll = dice_dist(dice_rng);
        const bool earned_another_turn = (roll == 6);

        // Delegate the move to the strategy
        strategy->make_move(game, player_id, roll);
        game.print_game_state(roll);

        // Check for win
        if (game.check_for_win(player_id)) {
            game.game_over = true;
            std::println("\n*** PLAYER {} HAS WON! ***", p_char);
        }

        // --- Pass turn ---
        if (!earned_another_turn || game.game_over) game.current_player = next_player(game.current_player);

        // Release lock and wake up all waiting threads
        lock.unlock();
        game.cv.notify_all();
    }
}