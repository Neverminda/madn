#pragma once

#include "PlayerStrategy.h"
#include <random>

/**
 * @brief Random move selection strategy.
 *
 * Chooses a random valid move from all available options.
 * Delegates move validation and execution to the Game class.
 */
class RandomStrategy : public PlayerStrategy {
private:
    std::mt19937 rng;

public:
    explicit RandomStrategy(int seed);

    bool make_move(Game& game, int player_id, int roll) override;
};
