#include "Game.h"
#include "constants.h"
#include <array>
#include <algorithm>
#include <print>
#include <string>

Game::Game()
    : pawn_positions()
    , current_player(PlayerID::A)
    , is_game_over(false)
    , rng(std::random_device{}())
{
    // All pawns start in the home
    for (auto &player_pawns: pawn_positions) {
        player_pawns.fill(POS_HOME);
        player_pawns[0] = POS_TRACK_START;
    }

    // Initialize position_lookup for the starting pawns
    for (int p = 0; p < 4; ++p) {
        const int abs_pos = PLAYER_START_SQUARE[p];
        position_lookup[abs_pos] = std::make_pair(p, 0);
    }
}

auto Game::check_for_win(const PlayerID player_id) const
-> bool
{
    return std::ranges::all_of(pawn_positions[to_int(player_id)], is_in_goal);
}

auto Game::print_game_state(const int roll) const
-> void
{
    std::string output;
    output.reserve(128);

    // 1. Home/Goal status for each player
    for (int p = 0; p < 4; ++p) {
        const auto home = static_cast<int>(std::ranges::count_if(pawn_positions[p], is_at_home));
        const auto goal = static_cast<int>(std::ranges::count_if(pawn_positions[p], is_in_goal));
        output += std::format("P{}(H:{},G:{}) ", to_char(to_player_id(p)), home, goal);
    }

    // 2. Track overview - use cached position_lookup for O(1) access
    std::array<char, TRACK_SIZE> track{};
    track.fill('.');

    output += "| Track: [";
    for (int pos = 0; pos < TRACK_SIZE; ++pos) {
        if (const auto& occupant = position_lookup[pos]) {
            const auto [player_idx, pawn_idx] = *occupant;
            const char player_char = to_char(to_player_id(player_idx));
            // 'X' if the field is occupied multiple times, otherwise player letter
            track[pos] = (track[pos] == '.' ? player_char : 'X');
        }
    }
    for (const char c : track) output += c;
    output += "] | ";

    // 3. Current player
    output += std::format("Turn: {} Roll: {}", to_char(current_player), roll);

    if constexpr (ENABLE_OUTPUT) std::println("{}", output);
}

auto Game::get_absolute_position(const PlayerID player, const int pawn_index) const
-> int
{
    const int player_idx = to_int(player);
    const int rel_pos = pawn_positions[player_idx][pawn_index];

    // Sanity check: On the main track?
    if (rel_pos < POS_TRACK_START || rel_pos >= POS_GOAL_START) return -1;

    return (PLAYER_START_SQUARE[player_idx] + rel_pos) % TRACK_SIZE;
}

auto Game::get_pawns_that_can_leave_home(const PlayerID player_id) const
-> std::vector<int>
{
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

auto Game::get_valid_moves_on_track(const PlayerID player_id, const int roll) const
-> std::vector<int>
{
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

auto Game::get_all_valid_moves(const PlayerID player_id, const int roll) const
-> std::vector<int>
{
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

auto Game::check_and_apply_capture(const PlayerID moving_player, const int landing_abs_pos)
-> void
{
    const int moving_player_idx = to_int(moving_player);

    // O(1) lookup: check if there's a pawn at the landing position
    if (const auto& occupant = position_lookup[landing_abs_pos]) {
        const auto [other_player_idx, other_pawn_idx] = *occupant;

        // Don't capture own pawns
        if (other_player_idx == moving_player_idx) return;

        // IMPORTANT: A player's start field is a safe zone, you cannot capture anyone on their own start
        if (landing_abs_pos == PLAYER_START_SQUARE[other_player_idx]) return;

        // Capture: send the pawn back to the home
        pawn_positions[other_player_idx][other_pawn_idx] = POS_HOME;
        // Clear from lookup (will be overwritten by the moving pawn anyway)
        position_lookup[landing_abs_pos] = std::nullopt;
    }
}

auto Game::update_position_lookup(const PlayerID player_id, const int pawn_index, const int old_rel_pos, const int new_rel_pos)
-> void
{
    const int player_idx = to_int(player_id);

    // Remove the old position from lookup (if it was on the main track)
    if (old_rel_pos >= POS_TRACK_START && old_rel_pos < POS_GOAL_START) {
        const int old_abs = (PLAYER_START_SQUARE[player_idx] + old_rel_pos) % TRACK_SIZE;
        position_lookup[old_abs] = std::nullopt;
    }

    // Add the new position to lookup (if it's on the main track)
    if (new_rel_pos >= POS_TRACK_START && new_rel_pos < POS_GOAL_START) {
        const int new_abs = (PLAYER_START_SQUARE[player_idx] + new_rel_pos) % TRACK_SIZE;
        position_lookup[new_abs] = std::make_pair(player_idx, pawn_index);
    }
}

auto Game::execute_move(const PlayerID player_id, const int pawn_index, const int roll)
-> void
{
    const int player_idx = to_int(player_id);

    if (const int old_pos = pawn_positions[player_idx][pawn_index]; is_at_home(old_pos)) {
        // Move pawn out of home
        pawn_positions[player_idx][pawn_index] = POS_TRACK_START;

        // Check if someone is captured on the start field
        const int abs_pos = get_absolute_position(player_id, pawn_index);
        check_and_apply_capture(player_id, abs_pos);

        // Update lookup table
        update_position_lookup(player_id, pawn_index, old_pos, POS_TRACK_START);
    } else {
        // Move pawn normally
        const int new_pos = old_pos + roll;
        pawn_positions[player_idx][pawn_index] = new_pos;

        // Update lookup table
        update_position_lookup(player_id, pawn_index, old_pos, new_pos);

        // Only capture if landing on the main track (not in goal)
        if (!is_in_goal(new_pos)) {
            const int abs_pos = get_absolute_position(player_id, pawn_index);
            check_and_apply_capture(player_id, abs_pos);
        }
    }
}

auto Game::announce_winner() const
-> void
{
    if constexpr (ENABLE_OUTPUT) {
        if (!winner) return;
        std::println("\n*** PLAYER {} HAS WON! ***\n", to_char(*winner));
    }
}

auto Game::roll_dice()
-> int
{
    std::uniform_int_distribution<int> dist(1, 6);
    return dist(rng);
}