#pragma once

#include <coroutine>
#include <exception>

/**
 * @brief A coroutine task type that represents an asynchronous operation.
 *
 * Task<void> provides implements the promise_type required for C++20 coroutines.
 */
class Task final {
public:
    /**
     * @brief Promise type required by the C++20 coroutine machinery.
     */
    struct promise_type {
        std::exception_ptr exception;

        /**
         * @brief Creates the Task object that will be returned to the caller.
         */
        auto get_return_object()
        -> Task { return Task{std::coroutine_handle<promise_type>::from_promise(*this)}; }

        /**
         * @brief Auto-starts the coroutine (eager execution).
         * Coroutine begins execution immediately and runs to the first suspension point.
         */
        static auto initial_suspend() noexcept
        -> std::suspend_never { return {}; }

        /**
         * @brief Suspends at the end to allow cleanup before destruction.
         */
        static auto final_suspend() noexcept
        -> std::suspend_always { return {}; }

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
     * @brief Constructs a Task from a coroutine handle.
     */
    explicit Task(const handle_type h) : coroutine_handle(h) {}

    /**
     * @brief Move constructor - transfers ownership of the coroutine handle.
     */
    Task(Task&& other) noexcept : coroutine_handle(other.coroutine_handle) { other.coroutine_handle = nullptr; }

    // Delete copy operations - Tasks are move-only
    Task(const Task&) = delete;
    auto operator=(const Task&)
    -> Task& = delete;

    /**
     * @brief Destroys the coroutine handle when the Task goes out of scope.
     */
    ~Task() {
        if (coroutine_handle) coroutine_handle.destroy();
    }

    /**
     * @brief Move assignment - transfers ownership of the coroutine handle.
     */
    auto operator=(Task&& other) noexcept
    -> Task&
    {
        if (this != &other) {
            if (coroutine_handle) coroutine_handle.destroy();
            coroutine_handle = other.coroutine_handle;
            other.coroutine_handle = nullptr;
        }
        return *this;
    }

    /**
     * @brief Resumes the coroutine if it's not done.
     */
    auto resume() const
    -> void
    {
        if (coroutine_handle && !coroutine_handle.done()) coroutine_handle.resume();
    }

    /**
     * @brief Checks if the coroutine has completed.
     */
    [[nodiscard]]
    auto done() const
    -> bool { return coroutine_handle.done(); }

    /**
     * @brief Gets the underlying coroutine handle.
     */
    [[nodiscard]]
    auto handle() const
    -> handle_type { return coroutine_handle; }

private:
    handle_type coroutine_handle;
};
