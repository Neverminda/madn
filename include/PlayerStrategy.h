#pragma once

#include "PlayerID.h"

class Game;

/**
 * @brief Strategy interface for player decision-making.
 *
 * Defines how a player chooses how to move during their turn.
 * Allows different AI implementations (i.e., random, smart, human, etc.)
 */
class PlayerStrategy {
public:
    virtual ~PlayerStrategy() = default;

    /**
     * @brief Executes one turn for the player.
     * @param game Reference to the game state
     * @param player_id The ID of the player
     * @param roll The dice roll result (1-6)
     * @return True if a move was made, false if no valid moves
     */
    virtual bool make_move(Game& game, PlayerID player_id, int roll) = 0;
};
