#pragma once

#include "strategies/PlayerStrategy.h"
#include "PlayerID.h"

class Game;

/**
 * @brief Random move selection strategy.
 *
 * Chooses a random valid move from all available options.
 * Delegates move validation and execution to the Game class.
 *
 * Compact implementation: stores only a seed (4 bytes) and call counter (4 bytes),
 * creating temporary RNG when needed. This reduces memory from 2504 bytes to just 8 bytes!
 * Satisfies CompactStrategy requirements for efficient value semantics.
 */
class RandomStrategy final {
public:
    explicit RandomStrategy(int seed);

    auto make_move(Game &game, PlayerID player_id, int roll) const
    -> bool;
private:
    int seed_;                    // Only 4 bytes - creates RNG on demand
    mutable int call_count_ = 0;  // Ensures different RNG state per call
};