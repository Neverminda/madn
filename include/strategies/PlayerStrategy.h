#pragma once

#include "PlayerID.h"
#include <concepts>

class Game;

/**
 * @brief Concept defining requirements for a PlayerStrategy type.
 *
 * A type satisfies PlayerStrategy if it provides a make_move method
 * that takes a Game reference, PlayerID, and dice roll, returning a bool.
 *
 * This enables compile-time polymorphism verification for the strategy pattern.
 * Replaces the traditional runtime polymorphism (virtual functions) with
 * zero-overhead compile-time polymorphism.
 */
template<typename T>
concept PlayerStrategy = requires(T strategy, Game& game, PlayerID player_id, int roll) {
    { strategy.make_move(game, player_id, roll) } -> std::same_as<bool>;
};

/**
 * @brief Concept for compact, efficient strategy implementations.
 *
 * A type satisfies CompactStrategy if it:
 * - Satisfies PlayerStrategy requirements
 * - Is no larger than 64 bytes (fits in CPU cache line)
 * - Has noexcept move constructor (no heap allocation on move)
 */
template<typename T>
concept CompactStrategy = PlayerStrategy<T> && sizeof(T) <= 64 && std::is_nothrow_move_constructible_v<T>;