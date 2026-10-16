/**
 * @file Stopwatch.ixx
 * @brief  A high-resolution timer based on std::chrono::steady_clock.
 */

module;

#include <chrono>

export module helios.core.time:Stopwatch;

export namespace helios::core::time {

/**
 * @brief A high-resolution timer based on std::chrono::steady_clock.
 *
 * Used to measure elapsed time intervals with the highest precision
 * provided by the OS (suitable for frame time measurements and profiling).
 */
class Stopwatch {

private:
    // Initialize to now so elapsed() is well-defined even if start() wasn't called explicitly.
    std::chrono::time_point<std::chrono::steady_clock> start_{std::chrono::steady_clock::now()};

public:
    /**
     * @brief Constructs and starts the stopwatch.
     *
     * The stopwatch is initialized to the current time so queries are
     * well-defined immediately after construction.
     */
    Stopwatch() noexcept = default;

    /**
     * @brief Starts (or restarts) the timer.
     * Captures the current timestamp.
     */
    void start() noexcept {
        start_ = std::chrono::steady_clock::now();
    }


    [[nodiscard]] std::chrono::nanoseconds::rep elapsedNanoseconds() const noexcept {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now() - start_
        ).count();
    }


    [[nodiscard]] std::chrono::milliseconds::rep elapsedMilliseconds() const noexcept {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start_
        ).count();
    }


    template<typename T = double>
    [[nodiscard]] T elapsedSeconds() const noexcept {
        return std::chrono::duration<T>(
            std::chrono::steady_clock::now() - start_
        ).count();
    }

};
} // namespace helios::core::time