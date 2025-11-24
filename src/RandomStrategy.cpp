#include "RandomStrategy.h"
#include "Game.h"

RandomStrategy::RandomStrategy(const int seed)
    : rng(seed) {}

bool RandomStrategy::make_move(Game& game, const int player_id, const int roll) {
    // Get all valid moves from the game

    // If there are valid moves, pick one randomly
    if (const auto valid_moves = game.get_all_valid_moves(player_id, roll); !valid_moves.empty()) {
        std::uniform_int_distribution<int> move_dist(0, static_cast<int>(valid_moves.size()) - 1);
        const int chosen_pawn = valid_moves[move_dist(rng)];

        // Execute the move via the game
        game.execute_move(player_id, chosen_pawn, roll);
        return true;
    }

    // No move possible, turn forfeited
    return false;
}
