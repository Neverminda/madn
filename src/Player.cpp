#include "Player.h"
#include "Game.h"
#include "PlayerStrategy.h"
#include <iostream>

Player::Player(const int id, std::unique_ptr<PlayerStrategy> strat)
    : player_id(id)
    , strategy(std::move(strat))
    , dice_dist(1, 6)
    , dice_rng(std::random_device{}() + id) {}

void Player::play_game(Game& game) {
    char p_char = 'A' + player_id;

    while (true) {
        // Acquire lock to access shared data
        std::unique_lock lock(game.mtx);

        // Wait until it's our turn OR the game is over
        game.cv.wait(lock, [&]() {
            return game.current_player == player_id || game.game_over;
        });

        // If the game is over, terminate thread
        if (game.game_over) {
            break;
        }

        // --- It's our turn ---
        const int roll = dice_dist(dice_rng);
        bool earned_another_turn = (roll == 6);

        // Delegate move decision to strategy
        strategy->make_move(game, player_id, roll);

        // Output game state (while holding lock!)
        game.print_game_state(roll);

        // Check for win
        if (game.check_for_win(player_id)) {
            game.game_over = true;
            std::cout << "\n\n*** PLAYER " << p_char << " HAS WON! ***" << std::endl;
        }

        // Pass turn
        if (!earned_another_turn || game.game_over) {
            game.current_player = (game.current_player + 1) % 4;
        }
        // (If 6 was rolled and game is not over, current_player stays the same)

        // Release lock and wake up all waiting threads
        lock.unlock();
        game.cv.notify_all();
    }
}