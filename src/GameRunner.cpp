#include "GameRunner.h"
#include "Game.h"
#include "Player.h"
#include "RandomStrategy.h"
#include "GameScheduler.h"
#include "Task.h"
#include <variant>
#include <vector>
#include <array>
#include <random>
#include <algorithm>
#include <ranges>
#include <chrono>

auto GameRunner::run(std::array<PlayerVariant, 4> players)
-> int
{
    Game game;
    GameScheduler scheduler{game};

    // Create tasks by visiting each variant and calling play_game()
    // std::visit handles the type dispatch at compile-time
    std::array<Task, 4> tasks = {
        std::visit([&](auto& player) {
            return player.play_game(game, scheduler);
        }, players[0]),

        std::visit([&](auto& player) {
            return player.play_game(game, scheduler);
        }, players[1]),

        std::visit([&](auto& player) {
            return player.play_game(game, scheduler);
        }, players[2]),

        std::visit([&](auto& player) {
            return player.play_game(game, scheduler);
        }, players[3])
    };

    // Run the game (same as run_random)
    scheduler.run_until_complete();
    game.announce_winner();

    return 0;
}

auto GameRunner::run_random()
    -> void {
    const std::array<PlayerVariant, 4> players = {
        Player{PlayerID::A, RandomStrategy{42}},
        Player{PlayerID::B, RandomStrategy{123}},
        Player{PlayerID::C, RandomStrategy{456}},
        Player{PlayerID::D, RandomStrategy{789}}
    };

    run(players);
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