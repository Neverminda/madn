#include "Game.h"
#include "constants.h"
#include <print>
#include <array>
#include <ranges>
#include <algorithm>

Game::Game()
    : pawn_positions()
    , current_player(PlayerID::A)
    , is_game_over(false)
{
    // All pawns start in the home
    for (auto &player_pawns: pawn_positions) {
        player_pawns.fill(POS_HOME);
        player_pawns[0] = POS_TRACK_START;
    }
}

bool Game::check_for_win(const PlayerID player_id) const {
    return std::ranges::all_of(pawn_positions[to_int(player_id)], is_in_goal);
}

void Game::print_game_state(const int roll) const {
    const char p_char = to_char(current_player);

    // 1. Home/Goal status for each player
    for (int p = 0; p < 4; ++p) {
        const auto home = static_cast<int>(std::ranges::count_if(pawn_positions[p], is_at_home));
        const auto goal = static_cast<int>(std::ranges::count_if(pawn_positions[p], is_in_goal));
        std::print("P{}(H:{},G:{}) ", to_char(to_player_id(p)), home, goal);
    }

    std::print("| Track: [");

    // 2. Track overview
    std::array<char, TRACK_SIZE> track{};
    track.fill('.');
    for (int p = 0; p < 4; ++p) {
        const char player_char = to_char(to_player_id(p));
        for (int i = 0; i < 4; ++i) {
            if (const int abs_pos = get_absolute_position(to_player_id(p), i); abs_pos != -1) {
                // 'X' if the field is occupied multiple times, otherwise player letter
                track[abs_pos] = (track[abs_pos] == '.' ? player_char : 'X');
            }
        }
    }
    for (const char c : track) std::print("{}", c);
    std::print("] | ");

    // 3. Current player
    std::println("Turn: {} Roll: {}", p_char, roll);
}

int Game::get_absolute_position(const PlayerID player, const int pawn_index) const {
    const int player_idx = to_int(player);
    const int rel_pos = pawn_positions[player_idx][pawn_index];

    // Sanity check: On the main track?
    if (rel_pos < POS_TRACK_START || rel_pos >= POS_GOAL_START) return -1;

    return (PLAYER_START_SQUARE[player_idx] + rel_pos) % TRACK_SIZE;
}

std::vector<int> Game::get_pawns_that_can_leave_home(const PlayerID player_id) const {
    std::vector<int> pawns;
    pawns.reserve(4);
    const int player_idx = to_int(player_id);

    // Check if own pawn blocks the start field. If free, find the first pawn in home
    const bool start_blocked = std::ranges::any_of(pawn_positions[player_idx], is_at_track_start);
    if (!start_blocked) {
        const auto it = std::ranges::find_if(pawn_positions[player_idx], is_at_home);
        if (it != pawn_positions[player_idx].end()) {
            pawns.push_back(static_cast<int>(std::distance(pawn_positions[player_idx].begin(), it)));
        }
    }

    return pawns;
}

std::vector<int> Game::get_valid_moves_on_track(const PlayerID player_id, const int roll) const {
    std::vector<int> pawns;
    pawns.reserve(4);
    const int player_idx = to_int(player_id);

    for (int i = 0; i < 4; ++i) {
        const int pos = pawn_positions[player_idx][i];
        if (is_at_home(pos)) continue; // Is in home (handled separately)

        const int new_pos = pos + roll;
        if (new_pos > POS_GOAL_END) continue; // Would overshoot the goal, illegal

        // Check if own pawn is blocking the target field
        const bool blocked_by_self = std::ranges::any_of(pawn_positions[player_idx], is_at_position(new_pos));
        if (!blocked_by_self) pawns.push_back(i);
    }

    return pawns;
}

std::vector<int> Game::get_all_valid_moves(const PlayerID player_id, const int roll) const {
    std::vector<int> valid_moves;
    valid_moves.reserve(8);

    // If rolled a 6, check pawns that can leave home
    if (roll == 6) valid_moves = get_pawns_that_can_leave_home(player_id);

    // Check pawns on the track that can move
    const int player_idx = to_int(player_id);
    for (int i = 0; i < 4; ++i) {
        const int pos = pawn_positions[player_idx][i];
        if (is_at_home(pos)) continue;

        const int new_pos = pos + roll;
        if (new_pos > POS_GOAL_END) continue;

        // Check if own pawn is blocking the target field
        const bool blocked_by_self = std::ranges::any_of(pawn_positions[player_idx], is_at_position(new_pos));
        if (!blocked_by_self) valid_moves.push_back(i);
    }

    return valid_moves;
}

void Game::check_and_apply_capture(const PlayerID moving_player, const int landing_abs_pos) {
    const int moving_player_idx = to_int(moving_player);

    for (int p = 0; p < 4; ++p) {
        if (p == moving_player_idx) continue; // Don't capture own pawns

        for (int i = 0; i < 4; ++i) {
            // Check if another player's pawn is on this field
            if (get_absolute_position(to_player_id(p), i) == landing_abs_pos) {
                // IMPORTANT: A player's start field is a safe zone, you cannot capture anyone on their own start
                if (landing_abs_pos != PLAYER_START_SQUARE[p]) pawn_positions[p][i] = POS_HOME; // Back to home
            }
        }
    }
}

void Game::execute_move(const PlayerID player_id, const int pawn_index, const int roll) {
    const int player_idx = to_int(player_id);

    if (const int old_pos = pawn_positions[player_idx][pawn_index]; is_at_home(old_pos)) {
        // Move pawn out of home
        pawn_positions[player_idx][pawn_index] = POS_TRACK_START;

        // Check if someone is captured on the start field
        const int abs_pos = get_absolute_position(player_id, pawn_index);
        check_and_apply_capture(player_id, abs_pos);
    } else {
        // Move pawn normally
        const int new_pos = old_pos + roll;
        pawn_positions[player_idx][pawn_index] = new_pos;

        // Only capture if landing on the main track (not in goal)
        if (!is_in_goal(new_pos)) {
            const int abs_pos = get_absolute_position(player_id, pawn_index);
            check_and_apply_capture(player_id, abs_pos);
        }
    }
}
