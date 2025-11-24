#pragma once

#include "PlayerStrategy.h"
#include <random>

/**
 * @brief Random move selection strategy.
 *
 * Chooses a random valid move from all available options.
 * Delegates move validation and execution to the Game class.
 */
class RandomStrategy final : public PlayerStrategy {
public:
    explicit RandomStrategy(int seed);

    bool make_move(Game& game, PlayerID player_id, int roll) override;
private:
    std::mt19937 rng;
};