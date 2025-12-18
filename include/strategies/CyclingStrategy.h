#pragma once

#include "strategies/PlayerStrategy.h"
#include "Generator.h"
#include "PlayerID.h"

class Game;

/**
 * @brief Cycling pawn selection strategy using Generator coroutine.
 *
 * Uses a Generator to cycle through pawn indices (0→1→2→3→0→...).
 * On each turn, tries pawns in cyclic order starting from the next index.
 *
 * Example behavior:
 * - Turn 1: Starts at 0, tries pawns in order: 0, 1, 2, 3
 * - Turn 2: Starts at 1, tries pawns in order: 1, 2, 3, 0
 * - Turn 3: Starts at 2, tries pawns in order: 2, 3, 0, 1
 * - Turn 4: Starts at 3, tries pawns in order: 3, 0, 1, 2
 *
 * Compact implementation: Only stores a Generator handle (8 bytes).
 * Satisfies CompactStrategy requirements for efficient value semantics.
 */
class CyclingStrategy final {
public:
    CyclingStrategy();

    auto make_move(Game &game, PlayerID player_id, int roll) const
    -> bool;

private:
    mutable Generator<int> cycle_generator_;  // mutable because const make_move needs to advance it
};
