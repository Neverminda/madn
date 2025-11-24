#pragma once

#include <array>
#include <mutex>
#include <condition_variable>

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
     * @brief Calculates the absolute board position (0-39) of a pawn.
     * @return Absolute position or -1 if in home/goal.
     */
    int get_absolute_position(int player, int pawn_index) const;

    /**
     * @brief Checks after a move whether an opponent's pawn was captured.
     * @param moving_player The player who just moved.
     * @param landing_abs_pos The absolute position where the pawn landed.
     */
    void check_and_apply_capture(int moving_player, int landing_abs_pos);

    /**
     * @brief Checks if a player has all pawns in the goal.
     */
    bool check_for_win(int player_id) const;

    /**
     * @brief Prints the entire game state in one line.
     */
    void print_game_state(int roll) const;
};
