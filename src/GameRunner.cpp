#include "GameRunner.h"
#include "Game.h"
#include "Player.h"
#include "RandomStrategy.h"
#include <thread>
#include <vector>
#include <array>
#include <random>
#include <algorithm>
#include <ranges>
#include <chrono>

auto GameRunner::run_random()
-> int
{
    Game game;

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

    game.announce_winner();

    return 0;
}

auto GameRunner::random_benchmark(const int num_games)
-> int
{
    std::println("=== Ludo Benchmark ===");
    std::println("Running {} games...\n", num_games);

    const auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_games; ++i) run_random();

    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Print statistics
    std::println("=== Benchmark Results ===");
    std::println("Total games:    {}", num_games);
    std::println("Total time:     {} ms", duration.count());
    std::println("Average time:   {:.2f} ms/game", static_cast<double>(duration.count()) / num_games);
    std::println("Games per sec:  {:.2f} games/s", (num_games * 1000.0) / static_cast<double>(duration.count()));

    return 0;
}
