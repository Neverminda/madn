#include "GameRunner.h"
#include "Game.h"
#include "Player.h"
#include "RandomStrategy.h"
#include "GameScheduler.h"
#include "Task.h"
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
    GameScheduler scheduler{game};

    // Create 4 players with random strategies
    using RandomPlayer = Player<RandomStrategy>;
    std::vector<RandomPlayer> players;
    players.reserve(4);
    players.emplace_back(PlayerID::A, RandomStrategy{static_cast<int>(std::random_device{}())});
    players.emplace_back(PlayerID::B, RandomStrategy{static_cast<int>(std::random_device{}())});
    players.emplace_back(PlayerID::C, RandomStrategy{static_cast<int>(std::random_device{}())});
    players.emplace_back(PlayerID::D, RandomStrategy{static_cast<int>(std::random_device{}())});

    // Create 4 coroutine tasks
    std::array tasks = {
        players[0].play_game(game, scheduler),
        players[1].play_game(game, scheduler),
        players[2].play_game(game, scheduler),
        players[3].play_game(game, scheduler)
    };

    // Run the game (single-threaded event loop)
    // Tasks are automatically destroyed at the end of their scope
    scheduler.run_until_complete();
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
