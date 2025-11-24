#pragma once

#include <array>

/**
 * @brief Enum representing the four players in the game.
 *
 * Type-safe alternative to using raw integers for player identification.
 */
enum class PlayerID { A = 0, B = 1, C = 2, D = 3 };

/**
 * @brief Converts a PlayerID to its corresponding character label.
 * @param id The player ID
 * @return Character 'A', 'B', 'C', or 'D'
 */
inline auto to_char(PlayerID id)
-> char {
    constexpr std::array<char, 4> labels = {'A', 'B', 'C', 'D'};
    return labels[static_cast<int>(id)];
}

/**
 * @brief Converts a PlayerID to its underlying integer value.
 * @param id The player ID
 * @return Integer value (0-3)
 */
inline auto to_int(PlayerID id)
-> int { return static_cast<int>(id); }

/**
 * @brief Converts an integer to a PlayerID.
 * @param value Integer value (0-3)
 * @return Corresponding PlayerID
 */
inline auto to_player_id(int value)
-> PlayerID { return static_cast<PlayerID>(value); }

/**
 * @brief Gets the next player in turn order.
 * @param current The current player
 * @return The next player (wraps around from D to A)
 */
inline auto next_player(PlayerID current)
-> PlayerID { return static_cast<PlayerID>((static_cast<int>(current) + 1) % 4); }
