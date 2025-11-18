#include <iostream>
#include <vector>
#include <array>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>
#include <chrono>
#include <string>
#include <sstream> // Für die Ausgabezeile
#include <print>

// --- Spielkonstanten ---

// Positionen der Spielfiguren
constexpr int POS_HOME = -1;      // Im Haus
constexpr int POS_TRACK_START = 0;  // Startfeld (relative Position 0)
//constexpr int POS_TRACK_END = 39;   // Letztes Feld vor dem Ziel (relative Position 39)
constexpr int POS_GOAL_START = 40;  // Erstes Zielfeld
constexpr int POS_GOAL_END = 43;    // Letztes Zielfeld
constexpr int TRACK_SIZE = 40;

// Die absoluten Startpositionen auf dem 40-Felder-Brett
const std::array<int, 4> PLAYER_START_SQUARE = {0, 10, 20, 30};

/**
 * @brief Enthält den gesamten geteilten Spielzustand und die Synchronisation.
 */
class Game {
public:
    // Speichert die Positionen aller Figuren: [Spieler][Figur_Index]
    std::array<std::array<int, 4>, 4> pawn_positions;
    int current_player;
    bool game_over;

    // Synchronisations-Tools
    std::mutex mtx;
    std::condition_variable cv;

    Game() : current_player(0), game_over(false) {
        // Alle Figuren starten im Haus
        for (auto& player_pawns : pawn_positions) {
            player_pawns.fill(POS_HOME);
            player_pawns[0] = POS_TRACK_START;
        }
    }

    /**
     * @brief Berechnet die absolute Brettposition (0-39) einer Figur.
     * @return Absolute Position oder -1, wenn im Haus/Ziel.
     */
    int get_absolute_position(int player, int pawn_index) const {
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
    void check_and_apply_capture(int moving_player, int landing_abs_pos) {
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
    bool check_for_win(int player_id) const {
        for (int i = 0; i < 4; ++i) {
            if (pawn_positions[player_id][i] < POS_GOAL_START) {
                return false; // Mindestens eine Figur noch nicht im Ziel
            }
        }
        return true;
    }

    /**
     * @brief Gibt den gesamten Spielzustand in einer Zeile aus.
     */
    void print_game_state(int roll) const {
        std::stringstream ss;
        char p_char = 'A' + current_player;

        // 1. Haus/Ziel-Status für jeden Spieler
        for (int p = 0; p < 4; ++p) {
            int home = 0;
            int goal = 0;
            for (int i = 0; i < 4; ++i) {
                if (pawn_positions[p][i] == POS_HOME) home++;
                else if (pawn_positions[p][i] >= POS_GOAL_START) goal++;
            }
            ss << "P" << (char)('A' + p) << "(H:" << home << ",G:" << goal << ") ";
        }

        ss << "| Track: [";

        // 2. Streckenübersicht
        std::array<char, TRACK_SIZE> track;
        track.fill('.');
        for (int p = 0; p < 4; ++p) {
            char player_char = 'A' + p;
            for (int i = 0; i < 4; ++i) {
                int abs_pos = get_absolute_position(p, i);
                if (abs_pos != -1) {
                    // 'X', wenn Feld mehrfach belegt, sonst Spielerbuchstabe
                    track[abs_pos] = (track[abs_pos] == '.' ? player_char : 'X');
                }
            }
        }
        for (char c : track) ss << c;
        ss << "] | ";

        // 3. Aktueller Spieler
        ss << "Zug: " << p_char << " Wurf: " << roll;

        // Mit \r an den Zeilenanfang springen und überschreiben.
        // Padding am Ende, um alte, längere Zeilen zu löschen.
        //std::cout << "\r" << ss.str() << "               " << std::flush;
        std::cout << ss.str() << std::endl;
    }
};


/**
 * @brief Die Hauptfunktion für jeden Spieler-Thread.
 */
void player_task(Game& game, int player_id) {
    // Jeder Thread erhält seinen eigenen Zufallszahlengenerator
    std::mt19937 rng(std::random_device{}() + player_id);
    std::uniform_int_distribution<int> dice_dist(1, 6);

    char p_char = 'A' + player_id;

    while (true) {
        // Sperre erwerben, um auf geteilte Daten zuzugreifen
        std::unique_lock<std::mutex> lock(game.mtx);
    
        // Warten, bis wir an der Reihe sind ODER das Spiel vorbei ist
        game.cv.wait(lock, [&]() {
            return game.current_player == player_id || game.game_over;
        });

        // Wenn das Spiel vorbei ist, Thread beenden
        if (game.game_over) {
            break;
        }

        // --- Wir sind am Zug ---
        int roll = dice_dist(rng);
        bool earned_another_turn = (roll == 6);

        // 1. Finde alle möglichen Züge
        std::vector<int> possible_move_pawns; // Speichert Indizes (0-3) der Figuren

        // 1a. Prüfen, ob wir aus dem Haus KÖNNEN (6 gewürfelt?)
        //bool can_move_from_home = false;
        if (roll == 6) {
            for (int i = 0; i < 4; ++i) {
                if (game.pawn_positions[player_id][i] == POS_HOME) {
                    // Prüfen, ob das eigene Startfeld (pos 0) frei ist
                    bool start_blocked = false;
                    for (int j = 0; j < 4; ++j) {
                        if (game.pawn_positions[player_id][j] == POS_TRACK_START) {
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
            int pos = game.pawn_positions[player_id][i];
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
                    if (game.pawn_positions[player_id][j] == new_pos) {
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

            int old_pos = game.pawn_positions[player_id][pawn_to_move_index];

            // Zug ausführen
            if (old_pos == POS_HOME) {
                // Figur aus dem Haus setzen
                game.pawn_positions[player_id][pawn_to_move_index] = POS_TRACK_START;
                // Prüfen, ob auf dem Startfeld jemand geschlagen wird
                int abs_pos = game.get_absolute_position(player_id, pawn_to_move_index);
                game.check_and_apply_capture(player_id, abs_pos);
            } else {
                // Figur normal bewegen
                int new_pos = old_pos + roll;
                game.pawn_positions[player_id][pawn_to_move_index] = new_pos;

                // Nur schlagen, wenn man auf der Hauptstrecke landet (nicht im Ziel)
                if (new_pos < POS_GOAL_START) {
                    int abs_pos = game.get_absolute_position(player_id, pawn_to_move_index);
                    game.check_and_apply_capture(player_id, abs_pos);
                }
            }
        }
        // else: Kein Zug möglich (z.B. alles blockiert), Zug verfällt.

        // 3. Spielzustand ausgeben (während Lock gehalten wird!)
        game.print_game_state(roll);

        // 4. Siegprüfung
        if (game.check_for_win(player_id)) {
            game.game_over = true;
            std::cout << "\n\n*** SPIELER " << p_char << " HAT GEWONNEN! ***" << std::endl;
        }

        // 5. Zug weitergeben
        if (!earned_another_turn || game.game_over) {
            game.current_player = (game.current_player + 1) % 4;
        }
        // (Wenn 6 gewürfelt wurde und Spiel nicht vorbei ist, bleibt current_player gleich)

        // Sperre freigeben und alle wartenden Threads aufwecken
        lock.unlock();
        
        game.cv.notify_all();

        // Kurze Pause, damit das Spiel sichtbar bleibt
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}


int one_game() {
    Game game;

    std::cout << "Mensch Ärgere dich nicht (Thread-Edition)" << std::endl;
    //std::cout << "Drücken Sie Enter, um das Spiel zu starten..." << std::endl;
    //std::cin.get();

    // Starte die 4 Spieler-Threads
    std::vector<std::thread> players;
    players.emplace_back(player_task, std::ref(game), 0); // Spieler A
    
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    players.emplace_back(player_task, std::ref(game), 1); // Spieler B
    players.emplace_back(player_task, std::ref(game), 2); // Spieler C
    players.emplace_back(player_task, std::ref(game), 3); // Spieler D

    // Warten, bis alle Threads (Spieler) fertig sind
    for (auto& t : players) {
        t.join();
    }

    std::cout << "Spiel beendet." << std::endl;
    return 0;
}

int main()
{
    one_game();
    return 0;
    
    for (int i=0; i<10; i++)
        one_game();
    auto start = std::chrono::steady_clock::now();
    for (int i=0; i<100; i++)
        one_game();
    auto end = std::chrono::steady_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::println("time {}", time);
}
