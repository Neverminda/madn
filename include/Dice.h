#pragma once

#include <concepts>
#include <random>

/**
 * @brief Concept for dice-like random number generators.
 *
 * A type satisfies DiceRollable if it:
 * - Can be called with operator() to produce an int result
 * - Provides min() and max() methods for range validation
 * - Has a valid result type that converts to int
 *
 * Enables the DI pattern for testing and flexibility for different probability distributions that can be added later
 * on.
 */
template<typename T>
concept DiceRollable = requires(T& dice) {
    { dice() }      -> std::convertible_to<int>;
    { dice.min() }  -> std::convertible_to<int>;
    { dice.max() }  -> std::convertible_to<int>;
};

/**
 * @brief Standard six-sided dice implementation.
 *
 * Wraps std::uniform_int_distribution<int> to provide a dice roller that produces values 1-6.
 * - Satisfies DiceRollable concept.
 */
class StandardDice final {
public:
    explicit StandardDice(std::mt19937& rng)
        : rng_(rng)
        , dist_(1, 6) {}

    // Function call operator
    auto operator()()
    -> int
    {
        return dist_(rng_);
    }

    static constexpr int min() { return 1; }
    static constexpr int max() { return 6; }

private:
    std::mt19937& rng_;
    std::uniform_int_distribution<int> dist_;
};

// Compile-time validation
static_assert(DiceRollable<StandardDice>, "StandardDice must satisfy DiceRollable");