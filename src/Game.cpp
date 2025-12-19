//
//  Game.hpp
//  madn
//
//  Created by Thomas Schmitt on 19.12.25.
//

#include <iostream>
#include <chrono>
#include <future>

#include "Game.hpp"
#include "statistics.hpp"

// Die absoluten Startpositionen auf dem 40-Felder-Brett
const std::array<int, 4> PLAYER_START_SQUARE = {0, 10, 20, 30};

/**
 * @brief Enthält den gesamten geteilten Spielzustand und die Synchronisation.
 */
Game::Game()
 : current_player(0)
 , game_over(false)
 , rng(std::random_device{}())
 , dice_dist(1, 6) {
	// Alle Figuren starten im Haus
	for (auto& player_pawns : pawn_positions) {
		player_pawns.fill(POS_HOME);
		player_pawns[0] = POS_TRACK_START;
	}

	stats.games++;
}

/**
 * @brief Berechnet die absolute Brettposition (0-39) einer Figur.
 * @return Absolute Position oder -1, wenn im Haus/Ziel.
 */
int Game::get_absolute_position(int player, int pawn_index) const {
	int rel_pos = pawn_positions[player][pawn_index];
	if (rel_pos < POS_TRACK_START || rel_pos >= POS_GOAL_START) {
		return -1; // Nicht auf der Hauptstrecke
	}
	return (PLAYER_START_SQUARE[player] + rel_pos) % TRACK_SIZE;
}

/**
 * @brief Prüft nach einem Zug, ob eine gegnerische Figur geschlagen wurde.
 * @param moving_player Der Spieler, der gerade gezogen hat.
 * @param landing_abs_pos Die absolute Position, auf der gelandet wurde.
 */
void Game::check_and_apply_capture(int moving_player, int landing_abs_pos) {
	for (int p = 0; p < 4; ++p) {
		if (p == moving_player) continue; // Eigene Figuren nicht schlagen

		for (int i = 0; i < 4; ++i) {
			// Prüfen, ob eine Figur eines anderen Spielers auf diesem Feld steht
			if (get_absolute_position(p, i) == landing_abs_pos) {

				// WICHTIG: Das Startfeld eines Spielers ist eine "sichere Zone"
				// Man kann niemanden auf seinem *eigenen* Startfeld schlagen.
				if (landing_abs_pos != PLAYER_START_SQUARE[p]) {
					pawn_positions[p][i] = POS_HOME; // Zurück ins Haus
				}
			}
		}
	}
}

/**
 * @brief Prüft, ob ein Spieler alle Figuren im Ziel hat.
 */
bool Game::check_for_win(int player_id) const {
	for (int i = 0; i < 4; ++i) {
		if (pawn_positions[player_id][i] < POS_GOAL_START) {
			return false; // Mindestens eine Figur noch nicht im Ziel
		}
	}
	return true;
}


void Game::player_task(int player_id) {

	char p_char = 'A' + player_id;

	// --- Wir sind am Zug ---
	int roll = dice_dist(rng);
	bool earned_another_turn = (roll == 6);

	// 1. Finde alle möglichen Züge
	std::vector<int> possible_move_pawns; // Speichert Indizes (0-3) der Figuren

	// 1a. Prüfen, ob wir aus dem Haus KÖNNEN (6 gewürfelt?)
	//bool can_move_from_home = false;
	if (roll == 6) {
		for (int i = 0; i < 4; ++i) {
			if (pawn_positions[player_id][i] == POS_HOME) {
				// Prüfen, ob das eigene Startfeld (pos 0) frei ist
				bool start_blocked = false;
				for (int j = 0; j < 4; ++j) {
					if (pawn_positions[player_id][j] == POS_TRACK_START) {
						start_blocked = true;
						break;
					}
				}
				if (!start_blocked) {
					possible_move_pawns.push_back(i); // Diese Figur (i) kann raus
					//can_move_from_home = true;
				}
				// Wir können nur EINE Figur aufs Startfeld setzen
				break;
			}
		}
	}

	// 1b. Prüfen, welche Figuren auf dem Feld bewegt werden können
	for (int i = 0; i < 4; ++i) {
		int pos = pawn_positions[player_id][i];
		if (pos == POS_HOME) continue; // Ist im Haus (schon oben behandelt)

		int new_pos = pos + roll;

		// Im Ziel
		if (new_pos > POS_GOAL_END) {
			continue; // Übers Ziel hinausgeschossen, ungültiger Zug
		}
		// Auf der Strecke oder im Ziel
		else {
			// Prüfen, ob das Zielfeld von eigener Figur blockiert ist
			bool blocked_by_self = false;
			for (int j = 0; j < 4; ++j) {
				if (i == j) continue;
				if (pawn_positions[player_id][j] == new_pos) {
					blocked_by_self = true;
					break;
				}
			}

			if (!blocked_by_self) {
				possible_move_pawns.push_back(i);
			}
		}
	}

	// 2. Führe einen zufälligen Zug aus (wenn möglich)
	if (!possible_move_pawns.empty()) {
		// Wähle einen zufälligen der möglichen Züge
		std::uniform_int_distribution<int> move_dist(0, (int)possible_move_pawns.size() - 1);
		int pawn_to_move_index = possible_move_pawns[move_dist(rng)];

		int old_pos = pawn_positions[player_id][pawn_to_move_index];

		// Zug ausführen
		if (old_pos == POS_HOME) {
			// Figur aus dem Haus setzen
			pawn_positions[player_id][pawn_to_move_index] = POS_TRACK_START;
			// Prüfen, ob auf dem Startfeld jemand geschlagen wird
			int abs_pos = get_absolute_position(player_id, pawn_to_move_index);
			check_and_apply_capture(player_id, abs_pos);
		} else {
			// Figur normal bewegen
			int new_pos = old_pos + roll;
			pawn_positions[player_id][pawn_to_move_index] = new_pos;

			// Nur schlagen, wenn man auf der Hauptstrecke landet (nicht im Ziel)
			if (new_pos < POS_GOAL_START) {
				int abs_pos = get_absolute_position(player_id, pawn_to_move_index);
				check_and_apply_capture(player_id, abs_pos);
			}
		}

		// X. Spielzustand speichern
		stats.save(player_id, roll, pawn_to_move_index, pawn_positions);
	}
	else {
		stats.save(player_id, roll, -1, pawn_positions);
	}
	// else: Kein Zug möglich (z.B. alles blockiert), Zug verfällt.

	// 3. Spielzustand ausgeben (während Lock gehalten wird!)
	if constexpr (false) print_game_state(roll);

	// 4. Siegprüfung
	if (check_for_win(player_id)) {
		game_over = true;
		//std::cout << "\n\n*** SPIELER " << p_char << " HAT GEWONNEN! ***" << std::endl;
	}

	// 5. Zug weitergeben
	if (!earned_another_turn || game_over) {
		current_player = (current_player + 1) % 4;
	}
	// (Wenn 6 gewürfelt wurde und Spiel nicht vorbei ist, bleibt current_player gleich)

	// Kurze Pause, damit das Spiel sichtbar bleibt
	//std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

/**
 * @brief Gibt den gesamten Spielzustand in einer Zeile aus.
 */
void Game::print_game_state(int roll) const {
	std::stringstream ss;
	ss << "P:" << char('A' + current_player) << " ";
	ss << "R:" << roll << " ";

	// --- 1. Häuser ---
	// Format: HA:xxxx HB:xxxx HC:xxxx HD:xxxx
	for (int p = 0; p < 4; ++p) {
		ss << "H" << char('A' + p) << ":";
		for (int i = 0; i < 4; ++i) {
			ss << (pawn_positions[p][i] == POS_HOME ? 'o' : '.');
		}
		ss << " ";
	}

	// --- 2. Strecke (40 Felder) ---
	// Zeichen:
	// .   = leer
	// A-D = 1 Figur
	// X   = mehrere Figuren
	ss << "T:";
	std::array<char, TRACK_SIZE> track;
	track.fill('.');

	for (int p = 0; p < 4; ++p) {
		char pc = 'A' + p;
		for (int i = 0; i < 4; ++i) {
			int abs_pos = get_absolute_position(p, i);
			if (abs_pos >= 0) {
				track[abs_pos] = (track[abs_pos] == '.' ? pc : 'X');
			}
		}
	}

	for (char c : track) ss << c;
	ss << " ";

	// --- 3. Ziele ---
	// Format: ZA:xxxx ZB:xxxx ZC:xxxx ZD:xxxx
	for (int p = 0; p < 4; ++p) {
		ss << "Z" << char('A' + p) << ":";
		for (int i = 0; i < 4; ++i) {
			bool is_goal = (pawn_positions[p][i] >= POS_GOAL_START);
			ss << (is_goal ? '*' : '.');
		}
		ss << " ";
	}

	std::cout << ss.str() << std::endl;
}

int Game::run() {
	int player = 0;
	while (!game_over) {
		player_task(player);
		player = ++player % 4;
	}

	//std::cout << "Spiel beendet." << std::endl;
	return 0;
}

statistics Game::runGames(int gameCount) {
	statistics stats;

	for (int i=0; i < gameCount; ++i) {
		Game game;
		game.run();
		stats.games += game.stats.games;
		stats.moves += game.stats.moves;
	}
	return stats;
}


statistics Game::runGamesInThreads(int gameCount, int threadCount) {
	std::vector<std::future<statistics>> games;
	statistics stats;

	const int gamePerThread = gameCount/threadCount;

	for (int i=0; i < threadCount; ++i) {
		games.emplace_back(std::async(Game::runGames, gamePerThread));
	}

	// Warten, bis alle Threads (Games) fertig sind
	for (auto& g : games) {
		auto gameStats = g.get();
		stats.games += gameStats.games;
		stats.moves += gameStats.moves;
	}

	//std::cout << "Spiel beendet." << std::endl;
	return stats;
}
