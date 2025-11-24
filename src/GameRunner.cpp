#include "GameRunner.h"
#include "Game.h"
#include "Player.h"
#include "RandomStrategy.h"
#include <thread>
#include <vector>
#include <array>
#include <random>
#include <iostream>
#include <print>
#include <algorithm>
#include <ranges>

int GameRunner::run_random() {
    Game game;

    std::println("Ludo Simulation with 4 players using the Random-Strategy");
    std::println("Press Enter to start the simulation...");
    std::cin.get();

    // Create 4 players with random strategies (compile-time polymorphism!)
    using RandomPlayer = Player<RandomStrategy>;
    std::vector<RandomPlayer> players;
    players.reserve(4);
    players.emplace_back(PlayerID::A, RandomStrategy{static_cast<int>(std::random_device{}())});
    players.emplace_back(PlayerID::B, RandomStrategy{static_cast<int>(std::random_device{}())});
    players.emplace_back(PlayerID::C, RandomStrategy{static_cast<int>(std::random_device{}())});
    players.emplace_back(PlayerID::D, RandomStrategy{static_cast<int>(std::random_device{}())});

    // Start player threads
    std::array<std::thread, 4> threads;
    auto indices = std::views::iota(size_t{0}, players.size());
    std::ranges::for_each(indices, [&](const size_t i) -> void {
        threads[i] = std::thread(&RandomPlayer::play_game, &players[i], std::ref(game));
    });

    // Wait until all threads (players) are finished
    std::ranges::for_each(threads, [](std::thread& t) -> void { t.join(); });

    std::println("Game ended.");
    return 0;
}
