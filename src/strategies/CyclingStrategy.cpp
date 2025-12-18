#include "strategies/CyclingStrategy.h"
#include "Game.h"
#include <algorithm>

/**
 * @brief Helper coroutine that creates an infinite cycle generator.
 * Yields pawn indices 0, 1, 2, 3, 0, 1, 2, 3, ... indefinitely.
 */
static auto create_cycle_generator()
-> Generator<int>
{
    while (true) {
        co_yield 0;
        co_yield 1;
        co_yield 2;
        co_yield 3;
    }
}

CyclingStrategy::CyclingStrategy()
    : cycle_generator_(create_cycle_generator())
{
    // Advance to the first value (generator starts suspended due to lazy evaluation)
    cycle_generator_.next();
}

auto CyclingStrategy::make_move(Game &game, const PlayerID player_id, const int roll) const
-> bool
{
    const int start_index = cycle_generator_.value();
    cycle_generator_.next();

    const auto valid_moves = game.get_all_valid_moves(player_id, roll);
    if (valid_moves.empty()) return false;

    // Try pawns in cyclic order starting from start_index
    // Example: if start_index = 2, tries: 2, 3, 0, 1
    for (int offset = 0; offset < 4; ++offset) {
        // Check if this pawn has a valid move
        const int pawn_index = (start_index + offset) % 4;
        if (std::ranges::find(valid_moves, pawn_index) != valid_moves.end()) {
            game.execute_move(player_id, pawn_index, roll);
            return true;
        }
    }

    // This should never happen if the implementation of get_all_valid_moves() is correct
    return false;
}

// Compile-time verification that CyclingStrategy satisfies the CompactStrategy concept
static_assert(CompactStrategy<CyclingStrategy>, "CyclingStrategy must satisfy CompactStrategy concept");
