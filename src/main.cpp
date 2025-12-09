#include "GameRunner.h"
#include "Player.h"
#include "strategies/RandomStrategy.h"
#include "strategies/CyclingStrategy.h"
#include <string>
#include <random>

auto main(const int argc, char* argv[])
-> int
{
    // Check if benchmark mode is requested
    if (argc > 1 && std::string(argv[1]) == "--benchmark") {
        const int num_games = (argc > 2) ? std::stoi(argv[2]) : 1000;
        return GameRunner::random_benchmark(num_games);
    }

    // Demonstrate mixed strategies: RandomStrategy and CyclingStrategy
    std::array<PlayerVariant, 4> players = {
        Player{PlayerID::A, RandomStrategy{42}},
        Player{PlayerID::B, CyclingStrategy{}},
        Player{PlayerID::C, CyclingStrategy{}},
        Player{PlayerID::D, RandomStrategy{123}}
    };

    return GameRunner::run(std::move(players));
}