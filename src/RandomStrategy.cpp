#include "RandomStrategy.h"
#include "Game.h"

RandomStrategy::RandomStrategy(const int seed)
    : rng(seed) {}

bool RandomStrategy::make_move(Game& game, const PlayerID player_id, const int roll) {
    // Some move(s) are possible, make a random valid move
    if (const auto valid_moves = game.get_all_valid_moves(player_id, roll); !valid_moves.empty()) {
        std::uniform_int_distribution move_dist(0, static_cast<int>(valid_moves.size()) - 1);
        const int chosen_pawn = valid_moves[move_dist(rng)];
        game.execute_move(player_id, chosen_pawn, roll);
        return true;
    }

    // No move possible, turn forfeited
    return false;
}
