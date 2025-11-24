#pragma once

class Game;

/**
 * @brief The main function for each player thread.
 */
void player_task(Game& game, int player_id);
