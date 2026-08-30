/**
 * @file JobSystem.ixx
 * @brief Thread-pool-based job scheduler with blocking wait semantics.
 */
module;

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

export module helios.core.thread.JobSystem;

export namespace helios::core::thread {

/**
 * @brief A fixed-size thread pool for parallel job execution.
 *
 * Maintains a pool of `std::jthread` workers that pull jobs from a shared
 * queue. `runAndWait()` enqueues a batch of jobs and blocks until every
 * job in that batch has completed.
 *
 * ### Example
 * ```cpp
 * JobSystem js(std::thread::hardware_concurrency());
 * std::vector<int> results(16);
 * js.runAndWait(results.size(), [&](std::size_t i) {
 *     results[i] = heavyCompute(i);
 * });
 * ```
 *
 * @note Only one `runAndWait()` call may be active at a time.
 *       Calling it concurrently from multiple threads is undefined behaviour.
 */
class JobSystem {

    std::queue<std::function<void()>> jobQueue_;
    std::mutex mutex_;
    std::condition_variable_any jobCondition_;
    std::condition_variable doneCondition_;

    /** @brief Maximum number of worker threads. */
    std::size_t maxWorkerCount_;

    /** @brief Number of jobs in the current batch that have not finished yet. */
    std::size_t pendingJobCount_ = 0;

    std::vector<std::jthread> workerThreads_;

public:
    /**
     * @brief Constructs the job system and spawns worker threads.
     *
     * @param maxWorkerCount Number of worker threads to create.
     *                       Typically `std::thread::hardware_concurrency()`.
     */
    explicit JobSystem(const std::size_t maxWorkerCount) : maxWorkerCount_(maxWorkerCount) {

        workerThreads_.reserve(maxWorkerCount);

        for (std::size_t i = 0; i < maxWorkerCount; ++i) {
            workerThreads_.emplace_back([this](const std::stop_token& stopToken) {
                while (!stopToken.stop_requested()) {
                    std::function<void()> job;

                    {
                        std::unique_lock lock(mutex_);

                        if (!jobCondition_.wait(lock, stopToken, [&] { return !jobQueue_.empty(); })) {
                            return;
                        }

                        job = std::move(jobQueue_.front());
                        jobQueue_.pop();
                    }

                    job();
                }
            });
        }
    }

    /**
     * @brief Enqueues `jobCount` jobs and blocks until all have finished.
     *
     * Calls `fn(i)` for each index `i` in `[0, jobCount)` on a worker
     * thread. Returns only after every invocation has completed.
     *
     * @tparam Fn   Callable type with signature `void(std::size_t)`.
     * @param jobCount  Number of parallel jobs to dispatch.
     * @param fn        Job body receiving the job index.
     *
     * @warning Must not be called concurrently from multiple threads.
     */
    template <typename Fn>
    void runAndWait(const std::size_t jobCount, Fn&& fn) {
        {
            std::scoped_lock lock(mutex_);

            pendingJobCount_ = jobCount;

            for (std::size_t i = 0; i < jobCount; ++i) {

                jobQueue_.emplace([this, i, &fn] {
                    fn(i);

                    {
                        std::scoped_lock lockInt(mutex_);
                        --pendingJobCount_;
                    }

                    doneCondition_.notify_one();
                });
            }
        }

        jobCondition_.notify_all();

        std::unique_lock lock(mutex_);

        doneCondition_.wait(lock, [&] { return pendingJobCount_ == 0; });
    }
};

} // namespace helios::core::thread
