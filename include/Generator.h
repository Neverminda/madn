#pragma once

#include <coroutine>
#include <exception>
#include <optional>

/**
 * @brief A generator coroutine type that produces a sequence of values.
 *
 * Generator<T> provides the promise_type required for C++20 coroutines
 * and supports co_yield for lazy value production.
 *
 * @tparam T The type of values yielded by the generator
 */
template<typename T>
class Generator final {
public:
    /**
     * @brief Promise type required by the C++20 coroutine machinery.
     */
    struct promise_type {
        std::optional<T> current_value;
        std::exception_ptr exception;

        /**
         * @brief Creates the Generator object that will be returned to the caller.
         */
        auto get_return_object()
        -> Generator { return Generator{std::coroutine_handle<promise_type>::from_promise(*this)}; }

        /**
         * @brief Lazy execution - suspends immediately without running.
         * Coroutine doesn't execute until first next() call.
         */
        static auto initial_suspend() noexcept
        -> std::suspend_always { return {}; }

        /**
         * @brief Suspends at the end to allow cleanup before destruction.
         */
        static auto final_suspend() noexcept
        -> std::suspend_always { return {}; }

        /**
         * @brief Called when the coroutine executes co_yield.
         * Stores the yielded value and suspends.
         */
        auto yield_value(T value) noexcept
        -> std::suspend_always {
            current_value = std::move(value);
            return {};
        }

        /**
         * @brief Called when the coroutine executes co_return.
         */
        static auto return_void()
        -> void {}

        /**
         * @brief Captures any unhandled exception from the coroutine.
         */
        auto unhandled_exception()
        -> void { exception = std::current_exception(); }
    };

    using handle_type = std::coroutine_handle<promise_type>;

    /**
     * @brief Constructs a Generator from a coroutine handle.
     */
    explicit Generator(const handle_type h)
        : coroutine_handle(h) {}

    /**
     * @brief Move constructor - transfers ownership of the coroutine handle.
     */
    Generator(Generator&& other) noexcept
        : coroutine_handle(other.coroutine_handle) { other.coroutine_handle = nullptr; }

    // Delete copy operations - Generators are move-only
    Generator(const Generator&) = delete;
    auto operator=(const Generator&)
    -> Generator& = delete;

    /**
     * @brief Destroys the coroutine handle when the Generator goes out of scope.
     */
    ~Generator() {
        if (coroutine_handle) coroutine_handle.destroy();
    }

    /**
     * @brief Move assignment - transfers ownership of the coroutine handle.
     */
    auto operator=(Generator&& other) noexcept
    -> Generator&
    {
        if (this != &other) {
            if (coroutine_handle) coroutine_handle.destroy();
            coroutine_handle = other.coroutine_handle;
            other.coroutine_handle = nullptr;
        }
        return *this;
    }

    /**
     * @brief Advances the generator to the next value.
     * @return true if a value was produced, false if the generator is exhausted
     */
    auto next()
    -> bool {
        if (!coroutine_handle || coroutine_handle.done()) {
            return false;
        }
        coroutine_handle.resume();
        return !coroutine_handle.done();
    }

    /**
     * @brief Gets the current yielded value.
     * @return Reference to the current value
     * @note Only valid after a successful next() call
     */
    [[nodiscard]]
    auto value() const
    -> const T& {
        return *coroutine_handle.promise().current_value;
    }

    /**
     * @brief Checks if the generator has been exhausted.
     */
    [[nodiscard]]
    auto done() const
    -> bool { return !coroutine_handle || coroutine_handle.done(); }

    /**
     * @brief Gets the underlying coroutine handle.
     */
    [[nodiscard]]
    auto handle() const
    -> handle_type { return coroutine_handle; }

private:
    handle_type coroutine_handle;
};
