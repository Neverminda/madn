//
//  Game.hpp
//  madn
//
//  Created by Thomas Schmitt on 19.12.25.
//

#include <array>
#include <mutex>
#include <random>
#include "statistics.hpp"

/**
 * @brief Enthält den gesamten geteilten Spielzustand und die Synchronisation.
 */
class Game {
private:

	// Positionen der Spielfiguren
	static constexpr int POS_HOME = -1;      // Im Haus
	static constexpr int POS_TRACK_START = 0;  // Startfeld (relative Position 0)
	//static constexpr int POS_TRACK_END = 39;   // Letztes Feld vor dem Ziel (relative Position 39)
	static constexpr int POS_GOAL_START = 40;  // Erstes Zielfeld
	static constexpr int POS_GOAL_END = 43;    // Letztes Zielfeld
	static constexpr int TRACK_SIZE = 40;

public:
    // Speichert die Positionen aller Figuren: [Spieler][Figur_Index]
    std::array<std::array<int, 4>, 4> pawn_positions; // 4x4x4 = 64 bytes !!!
    int current_player;
    bool game_over;

    // Synchronisations-Tools
	std::mt19937 rng;
	std::uniform_int_distribution<int> dice_dist;

	statistics stats;

	Game();

    /**
     * @brief Berechnet die absolute Brettposition (0-39) einer Figur.
     * @return Absolute Position oder -1, wenn im Haus/Ziel.
     */
	int get_absolute_position(int player, int pawn_index) const;

    /**
     * @brief Prüft nach einem Zug, ob eine gegnerische Figur geschlagen wurde.
     * @param moving_player Der Spieler, der gerade gezogen hat.
     * @param landing_abs_pos Die absolute Position, auf der gelandet wurde.
     */
	void check_and_apply_capture(int moving_player, int landing_abs_pos);

    /**
     * @brief Prüft, ob ein Spieler alle Figuren im Ziel hat.
     */
	bool check_for_win(int player_id) const;

    /**
     * @brief Gibt den gesamten Spielzustand in einer Zeile aus.
     */
	void print_game_state(int roll) const;

	void player_task(int player_id);

	int run();

	static statistics runGames(int gameCount);
	static statistics runGamesInThreads(int gameCount, int threadCount);

};
