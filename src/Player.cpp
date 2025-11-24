#include "Player.h"
#include "Game.h"
#include "constants.h"
#include <vector>
#include <random>
#include <iostream>

void player_task(Game& game, const int player_id) {
    // Each thread gets its own random number generator
    std::mt19937 rng(std::random_device{}() + player_id);
    std::uniform_int_distribution dice_dist(1, 6);

    const char p_char = 'A' + player_id;

    while (true) {
        // Acquire lock to access shared data
        std::unique_lock lock(game.mtx);

        // Wait until it's our turn OR the game is over
        game.cv.wait(lock, [&]() -> bool {
            return game.current_player == player_id || game.game_over;
        });

        // If the game is over, terminate the thread
        if (game.game_over) {
            break;
        }

        // --- It's our turn ---
        const int roll = dice_dist(rng);
        const bool earned_another_turn = (roll == 6);

        // 1. Find all possible moves
        std::vector<int> possible_move_pawns; // Stores indices (0-3) of pawns

        // 1a. Check if we CAN move out of home (rolled a 6?)
        if (roll == 6) {
            for (int i = 0; i < 4; ++i) {
                if (game.pawn_positions[player_id][i] == POS_HOME) {
                    // Check if our own start field (pos 0) is free
                    bool start_blocked = false;
                    for (int j = 0; j < 4; ++j) {
                        if (game.pawn_positions[player_id][j] == POS_TRACK_START) {
                            start_blocked = true;
                            break;
                        }
                    }
                    if (!start_blocked) {
                        possible_move_pawns.push_back(i); // This pawn (i) can move out
                    }
                    // We can only put ONE pawn on the start field
                    break;
                }
            }
        }

        // 1b. Check which pawns on the field can be moved
        for (int i = 0; i < 4; ++i) {
            const int pos = game.pawn_positions[player_id][i];
            if (pos == POS_HOME) continue; // Is in home (already handled above)

            // In goal
            if (const int new_pos = pos + roll; new_pos > POS_GOAL_END) {
                continue; // Overshot the goal, invalid move
            }
            // On track or in goal
            else {
                // Check if own pawn blocks the target field
                bool blocked_by_self = false;
                for (int j = 0; j < 4; ++j) {
                    if (i == j) continue;
                    if (game.pawn_positions[player_id][j] == new_pos) {
                        blocked_by_self = true;
                        break;
                    }
                }

                if (!blocked_by_self) {
                    possible_move_pawns.push_back(i);
                }
            }
        }

        // 2. Execute a random move (if possible)
        if (!possible_move_pawns.empty()) {
            // Choose a random one of the possible moves
            std::uniform_int_distribution<int> move_dist(0, static_cast<int>(possible_move_pawns.size()) - 1);
            const int pawn_to_move_index = possible_move_pawns[move_dist(rng)];

            // Execute move
            if (const int old_pos = game.pawn_positions[player_id][pawn_to_move_index]; old_pos == POS_HOME) {
                // Move pawn out of home
                game.pawn_positions[player_id][pawn_to_move_index] = POS_TRACK_START;
                // Check if someone is captured on the start field
                int abs_pos = game.get_absolute_position(player_id, pawn_to_move_index);
                game.check_and_apply_capture(player_id, abs_pos);
            } else {
                // Move pawn normally
                const int new_pos = old_pos + roll;
                game.pawn_positions[player_id][pawn_to_move_index] = new_pos;

                // Only capture if landing on the main track (not in goal)
                if (new_pos < POS_GOAL_START) {
                    const int abs_pos = game.get_absolute_position(player_id, pawn_to_move_index);
                    game.check_and_apply_capture(player_id, abs_pos);
                }
            }
        }
        // else: No move possible (e.g. everything blocked), turn forfeited.

        // 3. Output game state (while holding lock!)
        game.print_game_state(roll);

        // 4. Check for win
        if (game.check_for_win(player_id)) {
            game.game_over = true;
            std::cout << "\n\n*** PLAYER " << p_char << " HAS WON! ***" << std::endl;
        }

        // 5. Pass turn
        if (!earned_another_turn || game.game_over) {
            game.current_player = (game.current_player + 1) % 4;
        }
        // (If 6 was rolled and the game is not over, current_player stays the same)

        // Release lock and wake up all waiting threads
        lock.unlock();

        game.cv.notify_all();
    }
}