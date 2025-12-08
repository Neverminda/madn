#include "GameRunner.h"
#include "PlayerID.h"
#include "RandomStrategy.h"
#include <string>
#include <random>

int main(const int argc, char* argv[])
{
    // Check if benchmark mode is requested
    if (argc > 1 && std::string(argv[1]) == "--benchmark") {
        const int num_games = (argc > 2) ? std::stoi(argv[2]) : 1000;
        return GameRunner::random_benchmark(num_games);
    }

    GameRunner::run_random();

    return 0;
}