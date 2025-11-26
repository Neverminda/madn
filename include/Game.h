#pragma once

#include "constants.h"
#include "PlayerID.h"
#include <array>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <optional>
#include <random>

/**
 * @brief Contains the entire shared game state and synchronization.
 */
class Game final {
public:
    /**
     * @brief Stores the positions of all pawns: [Player][Pawn_Index]
     */
    std::array<std::array<int, 4>, 4> pawn_positions;

    /**
     * @brief Reverse lookup: maps absolute track position -> (player_idx, pawn_idx)
     * - Enables O(1) capture checking instead of O(n*m) nested loops
     */
    std::array<std::optional<std::pair<int, int>>, TRACK_SIZE> position_lookup;

    PlayerID current_player;
    bool is_game_over;
    std::optional<PlayerID> winner;

    // Synchronization tools
    std::mutex mtx;
    std::condition_variable cv;

    Game();

    /**
     * @brief Checks if a player has all pawns in the goal.
     */
    [[nodiscard]]
    auto check_for_win(PlayerID player_id) const
    -> bool;

    /**
     * @brief Prints the entire game state in one line.
     */
    auto print_game_state(int roll) const
    -> void;

    /**
     * @brief Calculates the absolute board position (0-39) of a pawn.
     * @return Absolute position or -1 if in home/goal.
     */
    [[nodiscard]]
    auto get_absolute_position(PlayerID player, int pawn_index) const
    -> int;

    /**
     * @brief Finds pawns that can leave home (requires rolling a 6).
     * @param player_id The player's ID
     * @return Vector of pawn indices that can leave home
     */
    [[nodiscard]]
    auto get_pawns_that_can_leave_home(PlayerID player_id) const
    -> std::vector<int>;

    /**
     * @brief Finds pawns on the track/goal that can move forward.
     * @param player_id The player's ID
     * @param roll The dice roll value
     * @return Vector of pawn indices that can move
     */
    [[nodiscard]]
    auto get_valid_moves_on_track(PlayerID player_id, int roll) const
    -> std::vector<int>;

    /**
     * @brief Gets all valid moves for a player given a die roll.
     * @param player_id The player's ID
     * @param roll The dice roll value
     * @return Vector of pawn indices that can be moved
     */
    [[nodiscard]]
    auto get_all_valid_moves(PlayerID player_id, int roll) const
    -> std::vector<int>;

    /**
     * @brief Checks after a move whether an opponent's pawn was captured.
     * @param moving_player The player who just moved.
     * @param landing_abs_pos The absolute position where the pawn landed.
     */
    auto check_and_apply_capture(PlayerID moving_player, int landing_abs_pos)
    -> void;

    /**
     * @brief Executes a move for a specific pawn.
     * @param player_id The player's ID
     * @param pawn_index Index of the pawn to move (0-3)
     * @param roll The dice roll value
     */
    auto execute_move(PlayerID player_id, int pawn_index, int roll)
    -> void;

    /**
     * @brief Prints the winner of the game.
     */
    auto announce_winner() const
    -> void;

    /**
     * @brief Rolls the dice (1-6).
     * @return The dice roll result
     */
    auto roll_dice()
    -> int;
private:
    /**
     * @brief Updates position_lookup when a pawn moves
     */
    auto update_position_lookup(PlayerID player_id, int pawn_index, int old_rel_pos, int new_rel_pos)
    -> void;

    [[nodiscard]]
    static constexpr auto is_in_goal(const int pos)
    -> bool { return pos >= POS_GOAL_START; }

    [[nodiscard]]
    static constexpr auto is_at_home(const int pos)
    -> bool { return pos == POS_HOME; }

    [[nodiscard]]
    static constexpr auto is_at_track_start(const int pos)
    -> bool { return pos == POS_TRACK_START; }

    [[nodiscard]]
    static auto is_at_position(int target_pos)
    -> std::function<bool(int)> { return [target_pos](const int pos) -> bool { return pos == target_pos;}; }

    std::mt19937 rng;
};
