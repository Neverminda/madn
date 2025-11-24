#include "Game.h"
#include "constants.h"
#include <iostream>
#include <sstream>

Game::Game() : pawn_positions(), current_player(0), game_over(false) {
    // All pawns start in the home
    for (auto &player_pawns: pawn_positions) {
        player_pawns.fill(POS_HOME);
        player_pawns[0] = POS_TRACK_START;
    }
}

int Game::get_absolute_position(const int player, const int pawn_index) const {
    const int rel_pos = pawn_positions[player][pawn_index];
    if (rel_pos < POS_TRACK_START || rel_pos >= POS_GOAL_START) {
        return -1; // Not on the main track
    }
    return (PLAYER_START_SQUARE[player] + rel_pos) % TRACK_SIZE;
}

void Game::check_and_apply_capture(const int moving_player, const int landing_abs_pos) {
    for (int p = 0; p < 4; ++p) {
        if (p == moving_player) continue; // Don't capture own pawns

        for (int i = 0; i < 4; ++i) {
            // Check if another player's pawn is on this field
            if (get_absolute_position(p, i) == landing_abs_pos) {

                // IMPORTANT: A player's start field is a "safe zone"
                // You cannot capture anyone on their *own* start field.
                if (landing_abs_pos != PLAYER_START_SQUARE[p]) {
                    pawn_positions[p][i] = POS_HOME; // Back to home
                }
            }
        }
    }
}

bool Game::check_for_win(const int player_id) const {
    for (int i = 0; i < 4; ++i) {
        if (pawn_positions[player_id][i] < POS_GOAL_START) {
            return false; // At least one pawn not yet in goal
        }
    }
    return true;
}

void Game::print_game_state(int roll) const {
    std::stringstream ss;
    const char p_char = 'A' + current_player;

    // 1. Home/Goal status for each player
    for (int p = 0; p < 4; ++p) {
        int home = 0;
        int goal = 0;
        for (int i = 0; i < 4; ++i) {
            if (pawn_positions[p][i] == POS_HOME) home++;
            else if (pawn_positions[p][i] >= POS_GOAL_START) goal++;
        }
        ss << "P" << static_cast<char>('A' + p) << "(H:" << home << ",G:" << goal << ") ";
    }

    ss << "| Track: [";

    // 2. Track overview
    std::array<char, TRACK_SIZE> track;
    track.fill('.');
    for (int p = 0; p < 4; ++p) {
        const char player_char = 'A' + p;
        for (int i = 0; i < 4; ++i) {
            const int abs_pos = get_absolute_position(p, i);
            if (abs_pos != -1) {
                // 'X' if field is occupied multiple times, otherwise player letter
                track[abs_pos] = (track[abs_pos] == '.' ? player_char : 'X');
            }
        }
    }
    for (char c : track) ss << c;
    ss << "] | ";

    // 3. Current player
    ss << "Turn: " << p_char << " Roll: " << roll;

    // Jump to line start with \r and overwrite.
    // Padding at end to clear old, longer lines.
    //std::cout << "\r" << ss.str() << "               " << std::flush;
    std::cout << ss.str() << std::endl;
}
