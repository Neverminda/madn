#pragma once

#include "PlayerID.h"
#include <array>
#include <mutex>
#include <condition_variable>
#include <vector>

/**
 * @brief Contains the entire shared game state and synchronization.
 */
class Game final {
public:
    // Stores the positions of all pawns: [Player][Pawn_Index]
    std::array<std::array<int, 4>, 4> pawn_positions;
    PlayerID current_player;
    bool game_over;

    // Synchronization tools
    std::mutex mtx;
    std::condition_variable cv;

    Game();

    /**
     * @brief Checks if a player has all pawns in the goal.
     */
    [[nodiscard]] bool check_for_win(PlayerID player_id) const;

    /**
     * @brief Prints the entire game state in one line.
     */
    void print_game_state(int roll) const;

    /**
     * @brief Calculates the absolute board position (0-39) of a pawn.
     * @return Absolute position or -1 if in home/goal.
     */
    [[nodiscard]] int get_absolute_position(PlayerID player, int pawn_index) const;

    /**
     * @brief Finds pawns that can leave home (requires rolling a 6).
     * @param player_id The player's ID
     * @return Vector of pawn indices that can leave home
     */
    [[nodiscard]] std::vector<int> get_pawns_that_can_leave_home(PlayerID player_id) const;

    /**
     * @brief Finds pawns on the track/goal that can move forward.
     * @param player_id The player's ID
     * @param roll The dice roll value
     * @return Vector of pawn indices that can move
     */
    [[nodiscard]] std::vector<int> get_valid_moves_on_track(PlayerID player_id, int roll) const;

    /**
     * @brief Gets all valid moves for a player given a dice roll.
     * @param player_id The player's ID
     * @param roll The dice roll value
     * @return Vector of pawn indices that can be moved
     */
    [[nodiscard]] std::vector<int> get_all_valid_moves(PlayerID player_id, int roll) const;

    /**
     * @brief Checks after a move whether an opponent's pawn was captured.
     * @param moving_player The player who just moved.
     * @param landing_abs_pos The absolute position where the pawn landed.
     */
    void check_and_apply_capture(PlayerID moving_player, int landing_abs_pos);

    /**
     * @brief Executes a move for a specific pawn.
     * @param player_id The player's ID
     * @param pawn_index Index of the pawn to move (0-3)
     * @param roll The dice roll value
     */
    void execute_move(PlayerID player_id, int pawn_index, int roll);
};
