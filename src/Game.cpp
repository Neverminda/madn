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

bool Game::check_for_win(const int player_id) const {
    for (int i = 0; i < 4; ++i) {
        if (pawn_positions[player_id][i] < POS_GOAL_START) {
            return false; // At least one pawn not yet in goal
        }
    }
    return true;
}

void Game::print_game_state(const int roll) const {
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
    std::array<char, TRACK_SIZE> track{};
    track.fill('.');
    for (int p = 0; p < 4; ++p) {
        const char player_char = 'A' + p;
        for (int i = 0; i < 4; ++i) {
            if (const int abs_pos = get_absolute_position(p, i); abs_pos != -1) {
                // 'X' if the field is occupied multiple times, otherwise player letter
                track[abs_pos] = (track[abs_pos] == '.' ? player_char : 'X');
            }
        }
    }
    for (const char c : track) ss << c;
    ss << "] | ";

    // 3. Current player
    ss << "Turn: " << p_char << " Roll: " << roll;
    std::cout << ss.str() << std::endl;
}

int Game::get_absolute_position(const int player, const int pawn_index) const {
    const int rel_pos = pawn_positions[player][pawn_index];
    if (rel_pos < POS_TRACK_START || rel_pos >= POS_GOAL_START) {
        return -1; // Not on the main track
    }
    return (PLAYER_START_SQUARE[player] + rel_pos) % TRACK_SIZE;
}

std::vector<int> Game::get_pawns_that_can_leave_home(const int player_id) const {
    std::vector<int> pawns;

    // Check if own pawn blocks the start field
    bool start_blocked = false;
    for (int j = 0; j < 4; ++j) {
        if (pawn_positions[player_id][j] == POS_TRACK_START) {
            start_blocked = true;
            break;
        }
    }

    // If start is free, find the first pawn in home
    if (!start_blocked) {
        for (int i = 0; i < 4; ++i) {
            if (pawn_positions[player_id][i] == POS_HOME) {
                pawns.push_back(i);
                break; // We can only put ONE pawn on the start field at a time
            }
        }
    }

    return pawns;
}

std::vector<int> Game::get_valid_moves_on_track(const int player_id, const int roll) const {
    std::vector<int> pawns;

    for (int i = 0; i < 4; ++i) {
        int pos = pawn_positions[player_id][i];
        if (pos == POS_HOME) continue; // Is in home (handled separately)

        int new_pos = pos + roll;
        if (new_pos > POS_GOAL_END) continue; // Would overshoot the goal, illegal

        // Check if own pawn is blocking the target field
        bool blocked_by_self = false;
        for (int j = 0; j < 4; ++j) {
            if (i == j) continue;
            if (pawn_positions[player_id][j] == new_pos) {
                blocked_by_self = true;
                break;
            }
        }

        if (!blocked_by_self) pawns.push_back(i);
    }

    return pawns;
}

std::vector<int> Game::get_all_valid_moves(const int player_id, const int roll) const {
    std::vector<int> valid_moves;

    // If rolled a 6, check pawns that can leave home
    if (roll == 6) {
        valid_moves = get_pawns_that_can_leave_home(player_id);
    }

    // Check pawns on the track that can move
    std::vector<int> track_moves = get_valid_moves_on_track(player_id, roll);
    valid_moves.insert(valid_moves.end(), track_moves.begin(), track_moves.end());

    return valid_moves;
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

void Game::execute_move(const int player_id, const int pawn_index, const int roll) {
    if (const int old_pos = pawn_positions[player_id][pawn_index]; old_pos == POS_HOME) {
        // Move pawn out of home
        pawn_positions[player_id][pawn_index] = POS_TRACK_START;

        // Check if someone is captured on the start field
        const int abs_pos = get_absolute_position(player_id, pawn_index);
        check_and_apply_capture(player_id, abs_pos);
    } else {
        // Move pawn normally
        const int new_pos = old_pos + roll;
        pawn_positions[player_id][pawn_index] = new_pos;

        // Only capture if landing on the main track (not in goal)
        if (new_pos < POS_GOAL_START) {
            const int abs_pos = get_absolute_position(player_id, pawn_index);
            check_and_apply_capture(player_id, abs_pos);
        }
    }
}
