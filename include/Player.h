#pragma once

#include <memory>
#include <random>

class Game;
class PlayerStrategy;

/**
 * @brief Represents a player in the game.
 *
 * Manages player state and coordinates with a strategy for decision-making.
 * Each player runs in its own thread and takes turns based on game state.
 */
class Player {
private:
    int player_id;
    std::unique_ptr<PlayerStrategy> strategy;
    std::uniform_int_distribution<int> dice_dist;
    std::mt19937 dice_rng;

public:
    /**
     * @brief Constructs a player with a given strategy.
     * @param id Player ID (0-3)
     * @param strat Strategy for move selection (takes ownership)
     */
    Player(int id, std::unique_ptr<PlayerStrategy> strat);

    /**
     * @brief Main game loop for this player (runs in a thread).
     * @param game Reference to the shared game state
     */
    void play_game(Game& game);

    int get_id() const { return player_id; }
};
