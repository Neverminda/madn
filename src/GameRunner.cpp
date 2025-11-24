#include "GameRunner.h"
#include "Game.h"
#include "Player.h"
#include <thread>
#include <vector>
#include <iostream>

int GameRunner::run() {
    Game game;

    std::cout << "Ludo Simulation in C++" << std::endl;
    std::cout << "Press Enter to start the simulation..." << std::endl;
    std::cin.get();

    // Start the 4 different player threads
    std::vector<std::thread> players;
    players.emplace_back(player_task, std::ref(game), 0);
    players.emplace_back(player_task, std::ref(game), 1);
    players.emplace_back(player_task, std::ref(game), 2);
    players.emplace_back(player_task, std::ref(game), 3);

    // Wait until all threads (players) are finished
    for (auto& t : players) {
        t.join();
    }

    std::cout << "Game ended." << std::endl;
    return 0;
}
