#include "GameRunner.h"
#include "Game.h"
#include "Player.h"
#include "RandomStrategy.h"
#include <thread>
#include <vector>
#include <memory>
#include <random>
#include <iostream>

int GameRunner::run() {
    Game game;

    std::cout << "Ludo Simulation in C++" << std::endl;
    std::cout << "Press Enter to start the simulation..." << std::endl;
    std::cin.get();

    // Create 4 players with random strategies
    std::vector<Player> players;
    for (int i = 0; i < 4; ++i) {
        players.emplace_back(i, std::make_unique<RandomStrategy>(std::random_device{}() + i));
    }

    // Start player threads
    std::vector<std::thread> threads;
    for (auto& player : players) {
        threads.emplace_back(&Player::play_game, &player, std::ref(game));
    }

    // Wait until all threads (players) are finished
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "Game ended." << std::endl;
    return 0;
}
