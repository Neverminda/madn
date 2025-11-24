#pragma once

#include <array>
#include <mutex>
#include <condition_variable>
#include <vector>

/**
 * @brief Contains the entire shared game state and synchronization.
 */
class Game {
public:
    // Stores the positions of all pawns: [Player][Pawn_Index]
    std::array<std::array<int, 4>, 4> pawn_positions;
    int current_player;
    bool game_over;

    // Synchronization tools
    std::mutex mtx;
    std::condition_variable cv;

    Game();

    /**
     * @brief Checks if a player has all pawns in the goal.
     */
    bool check_for_win(int player_id) const;

    /**
     * @brief Prints the entire game state in one line.
     */
    void print_game_state(int roll) const;

    /**
     * @brief Calculates the absolute board position (0-39) of a pawn.
     * @return Absolute position or -1 if in home/goal.
     */
    int get_absolute_position(int player, int pawn_index) const;

    /**
     * @brief Finds pawns that can leave home (requires rolling a 6).
     * @param player_id The player's ID
     * @return Vector of pawn indices that can leave home
     */
    std::vector<int> get_pawns_that_can_leave_home(int player_id) const;

    /**
     * @brief Finds pawns on the track/goal that can move forward.
     * @param player_id The player's ID
     * @param roll The dice roll value
     * @return Vector of pawn indices that can move
     */
    std::vector<int> get_valid_moves_on_track(int player_id, int roll) const;

    /**
     * @brief Gets all valid moves for a player given a dice roll.
     * @param player_id The player's ID
     * @param roll The dice roll value
     * @return Vector of pawn indices that can be moved
     */
    std::vector<int> get_all_valid_moves(int player_id, int roll) const;

    /**
     * @brief Checks after a move whether an opponent's pawn was captured.
     * @param moving_player The player who just moved.
     * @param landing_abs_pos The absolute position where the pawn landed.
     */
    void check_and_apply_capture(int moving_player, int landing_abs_pos);

    /**
     * @brief Executes a move for a specific pawn.
     * @param player_id The player's ID
     * @param pawn_index Index of the pawn to move (0-3)
     * @param roll The dice roll value
     */
    void execute_move(int player_id, int pawn_index, int roll);
};
