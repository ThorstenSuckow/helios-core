module;

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <future>
#include <latch>
#include <mutex>
#include <thread>
#include <vector>
#include <type_traits>
#include <chrono>


export module helios.core.thread.ThreadPool;

import helios.core.thread.ThreadSafeQueue;
import :WorkStealingQueue;
import :FunctionWrapper;
import :JoinThreads;

export namespace helios::core::thread {

    /**
     * @brief Thread pool with work stealing and task-group synchronization.
     *
     * @details
     * The pool supports nested task parallelism. Tasks submitted from outside
     * a worker thread are placed in the global pool queue. Tasks submitted by
     * a worker thread are placed in that worker's local work-stealing queue.
     *
     * Consider a parallel schedule S1 containing three sequential system groups:
     *
     * S1
     *  ├─ G1
     *  │  ├─ s1
     *  │  ├─ s2
     *  │  └─ s3
     *  ├─ G2
     *  │  └─ s4
     *  └─ G3
     *     ├─ s5
     *     └─ s6
     *
     * The scheduler invokes runAndWait() with three tasks, one for each group:
     *
     *     runAndWait(3, [(s1, s2, s3), (s4), (s5, s6)]);
     *
     * Each group task executes its systems sequentially, while the groups
     * themselves may execute concurrently:
     *
     *     w0:  s1  ->  s2  ->  s3
     *                  └─ runAndWait(2, [(t1)], [(t2)])
     *     w1:  s4
     *     w2:  s5  ->  s6
     *
     * The latch associated with this runAndWait() invocation reaches zero once
     * all three group tasks have completed. While waiting, the calling thread
     * repeatedly executes available tasks or yields when no work is available.
     *
     * Idle worker threads initially poll for pending tasks. After
     * waitBeforeSleep_ runs out with unsuccessful attempts, they wait on a condition variable.
     *
     * A sleeping worker is only notified if more tasks are pending than threads are
     * currently searching for work. Searching threads are idle workers in their polling
     * phase and callers of runAndWait() waiting for their task group; threads executing
     * a task do not count as searching. The check is performed when a task is submitted
     * and whenever a searching thread takes a task, so that the number of awake workers
     * grows step by step with the number of pending tasks, while no worker is woken if
     * the searching threads suffice.
     *
     * At most maxPolling_ idle workers poll simultaneously. An idle worker has to reserve
     * one of these polling slots; a worker that does not obtain a slot goes to sleep
     * immediately. Without this limit, several polling workers share the few available
     * tasks, each of them repeatedly resets its idle timer, and none of them ever reaches
     * waitBeforeSleep_. The limit only restricts idle workers, not the number of workers
     * executing tasks: a worker taking a task releases its slot, which allows further
     * workers to be woken if more tasks are pending.
     *
     * This hybrid polling and sleeping strategy reduces CPU utilization during
     * idle periods while avoiding the wake-up overhead for short idle intervals.
     *
     * A worker searches for pending work in the following order:
     *
     *   - local queue:
     *       tasks created by the currently executing worker, e.g. through nested
     *       runAndWait() calls for intra-system/data parallelism (e.g. t1, t2 created by s2)
     *
     *   - pool queue:
     *       tasks submitted from threads that are not workers of this pool;
     *
     *   - other worker queues:
     *       pending tasks stolen from another worker's local queue.
     *
     * For example, after w1 has completed G2, it cannot take over s2 from G1,
     * because G1 is already executing as one task (s1, s2, s3) on w0. However,
     * since s2 created (t1), (t2) as additional child tasks through a nested runAndWait(),
     * those pending child tasks are placed in w0's local queue, which may be stolen and
     * executed by w1.
     *
     * Each runAndWait() invocation owns its own latch. Child tasks reference the
     * latch of the task group that created them, irrespective of which worker
     * eventually executes them. Nested runAndWait() calls therefore form nested
     * synchronization scopes while all tasks share the same worker pool.
     *
     * @see Williams, Anthony: Concurrency in Action, 2nd; Manning Publications [pp. 315]
     */
    class ThreadPool {

    private:

        using TaskType =  FunctionWrapper;
        std::atomic<bool> running_ = true;

        /**
         * @brief Number of milliseconds a worker performs (yielding) before it goes to sleep.
         *
         * Keeps wake-up latency low for tasks that are submitted in quick succession
         * (e.g. consecutive parallel system groups), while idle workers stop consuming CPU.
         * Align to expected fps to prevent workers from successively being put to sleep and
         * woken up, which may cost unnecessary system calls.
         */
        std::chrono::milliseconds waitBeforeSleep_{20};

        std::size_t spinDuration_{1024};

        /**
         * @brief Number of tasks that were submitted but not yet taken for execution.
         */
        std::atomic<std::size_t> pendingTasks_{0};

        /**
         * @brief Number of threads currently searching for work.
         *
         * Counts idle workers in their polling phase and callers of runAndWait()
         * waiting for their task group. Threads executing a task are not counted.
         */
        std::atomic<std::size_t> searching_{0};

        /**
         * @brief Number of workers waiting on wakeCv_.
         */
        std::atomic<std::size_t> sleeping_{0};

        /**
         * @brief Number of reserved polling slots, i.e. idle workers in their polling phase.
         *
         * Unlike searching_, callers of runAndWait() are not counted.
         * Invariant: 0 <= polling_ <= maxPolling_.
         */
        std::atomic<std::size_t> polling_{0};

        /**
         * @brief Maximum number of idle workers that poll for tasks simultaneously.
         */
        std::size_t maxPolling_{1};

        std::mutex sleepMutex_;
        std::condition_variable wakeCv_;

        ThreadSafeQueue<TaskType> poolWorkQueue_;

        std::vector<std::unique_ptr<WorkStealingQueue>> queues_;

        std::vector<std::thread> threads_;

        JoinThreads joinThreads_;

        inline static thread_local  WorkStealingQueue* localWorkQueue_;
        inline static thread_local  std::size_t localQueueIndex_;
        inline static thread_local  std::size_t localThreadId_;

        /**
         * @brief Wakes one sleeping worker if more tasks are pending than threads are searching.

         */
        void wakeIfNeeded() {
            if (sleeping_.load() > 0 && pendingTasks_.load() > searching_.load()) {
                wakeWorkers(false);
            }
        }

        /**
         * @brief Executes a task taken by a searching thread.
         *
         * The thread stops searching for the duration of the task. Since this may leave
         * pending tasks without a searching thread, another worker is woken if required.
         */
        void runAsSearcher(TaskType& task) {
            pendingTasks_.fetch_sub(1);
            searching_.fetch_sub(1);
            wakeIfNeeded();
            task();
            searching_.fetch_add(1);
        }

        /**
         * @brief Reserves a polling slot if fewer than maxPolling_ slots are taken.
         *
         * @return true if a slot was reserved; the caller must release it with
         *         polling_.fetch_sub(1) when it stops polling.
         */
        [[nodiscard]] bool tryAcquirePollingSlot() noexcept {
            auto current = polling_.load();
            while (current < maxPolling_) {
                if (polling_.compare_exchange_weak(current, current + 1)) {
                    return true;
                }
            }
            return false;
        }

        void workerThread(const unsigned int threadId) {
            localQueueIndex_ = threadId;
            localThreadId_ = threadId + 1;
            localWorkQueue_ = queues_[localQueueIndex_].get();

            using Clock =  std::chrono::steady_clock;
            bool waitPhaseStarted = false;
            auto idleSince = Clock::now();

            std::size_t spinCount = 0;

            searching_.fetch_add(1);
            bool hasPollingSlot = tryAcquirePollingSlot();

            while (running_) {
                if (TaskType task; tryPopTask(task)) {
                    if (hasPollingSlot) {
                        polling_.fetch_sub(1);
                        hasPollingSlot = false;
                    }
                    runAsSearcher(task);
                    hasPollingSlot = tryAcquirePollingSlot();
                    spinCount = 0;
                    waitPhaseStarted = false;
                    continue;
                }

                // Only the holder of a polling slot polls; all other idle workers
                // (including all but maxPolling_ workers right after pool construction)
                // go to sleep immediately.
                if (hasPollingSlot) {
                    if (spinCount < spinDuration_) {
                        ++spinCount;
                        continue;
                    }

                    if (!waitPhaseStarted) {
                        waitPhaseStarted = true;
                        idleSince = Clock::now();
                    }

                    if (Clock::now() - idleSince < waitBeforeSleep_) {
                        std::this_thread::yield();
                        continue;
                    }

                    polling_.fetch_sub(1);
                    hasPollingSlot = false;
                }

                // Sleep until work is submitted or the pool shuts down.
                // searching_ is decremented before the predicate reads pendingTasks_;
                // see wakeIfNeeded() for why no wake-up can be lost.
                searching_.fetch_sub(1);
                {
                    std::unique_lock<std::mutex> lock(sleepMutex_);
                    sleeping_.fetch_add(1);
                    wakeCv_.wait(lock, [this] {
                        return !running_ || pendingTasks_.load() > 0;
                    });
                    sleeping_.fetch_sub(1);
                }
                searching_.fetch_add(1);
                hasPollingSlot = tryAcquirePollingSlot();

                spinCount = 0;
                waitPhaseStarted = false;
            }

            if (hasPollingSlot) {
                polling_.fetch_sub(1);
            }
            searching_.fetch_sub(1);
        }

        void wakeWorkers(const bool all) {
            // Acquiring the mutex orders this notification after a worker
            // that is between its predicate check and wait().
            { std::lock_guard<std::mutex> lock(sleepMutex_); }
            if (all) {
                wakeCv_.notify_all();
            } else {
                wakeCv_.notify_one();
            }
        }

        bool tryPopTask(TaskType& task) {
            return popTaskFromLocalQueue(task) ||
                   popTaskFromPoolQueue(task) ||
                   popTaskFromOtherThreadQueue(task);
        }

        bool popTaskFromLocalQueue(TaskType& task) {
            return localWorkQueue_ && localWorkQueue_->tryPop(task);
        }

        bool popTaskFromPoolQueue(TaskType& task) {
            return poolWorkQueue_.tryPop(task);
        }

        bool popTaskFromOtherThreadQueue(TaskType& task) {

            for (unsigned i = 0; i < queues_.size(); ++i) {
                unsigned const threadId = (localQueueIndex_ + i + 1) % queues_.size();
                if (queues_[threadId]->trySteal(task)) {
                    return true;
                }
            }
            return false;
        }


    public:

        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;
        ThreadPool(ThreadPool&&) = delete;
        ThreadPool& operator=(ThreadPool&&) = delete;

        /**
         * @brief Default time in ms the worker polls (yielding) before an idle worker goes to sleep.
         */
        static constexpr std::chrono::milliseconds DEFAULT_WAIT_BEFORE_SLEEP{20};

        static std::size_t currentThreadId() noexcept {
            return localThreadId_;
        }

        /**
         * @param numThreads Number of worker threads. The calling thread of runAndWait()
         *        additionally executes tasks while it waits.
         * @param waitBeforeSleep Time in ms the worker polls (yielding) before an idle worker
         *        sleeps on a condition variable.
         */
        explicit ThreadPool(
            const std::size_t numThreads,
            const std::chrono::milliseconds waitBeforeSleep = DEFAULT_WAIT_BEFORE_SLEEP
        )
            : running_(true),
            waitBeforeSleep_(waitBeforeSleep),
            joinThreads_(threads_) {

            queues_.reserve(numThreads);
            threads_.reserve(numThreads);

            try {
                for (unsigned i = 0; i < numThreads; ++i) {
                    queues_.emplace_back(std::make_unique<WorkStealingQueue>());
                }
                for (unsigned i = 0; i < numThreads; ++i) {
                    threads_.emplace_back(&ThreadPool::workerThread, this, i);
                }
            } catch (...) {
                running_ = false;
                wakeWorkers(true);
                throw;
            }
        }

        ~ThreadPool() {
            running_ = false;
            // sleeping workers must be woken up, otherwise join() would block forever
            wakeWorkers(true);
            for (auto& thread : threads_) {
                if (thread.joinable()) {
                    thread.join();
                }
            }
        }


        template<typename FunctionType>
        std::future<typename std::invoke_result_t<FunctionType>> submit(FunctionType f) {
            using ResultType = typename std::invoke_result_t<FunctionType>;

            std::packaged_task<ResultType()> packagedTask(std::move(f));
            std::future<ResultType> future(packagedTask.get_future());

            TaskType task{std::move(packagedTask)};

            // must be incremented before the task becomes visible to other threads
            pendingTasks_.fetch_add(1);

            if (localWorkQueue_) {
                localWorkQueue_->push(std::move(task));
            } else {
                poolWorkQueue_.push(std::move(task));
            }

            wakeIfNeeded();
            return future;
        }

        template<typename FunctionType>
        void runAndWait(unsigned int numTasks, FunctionType f) {
            std::latch done{numTasks};

            std::mutex exceptionMutex;
            std::exception_ptr exception;

            // The caller searches for (its own) tasks while waiting for the group.
            searching_.fetch_add(1);

            for (std::size_t i = 0; i < numTasks; ++i) {
                submit([&, i]() {
                    try {
                        f(i);
                    } catch (...) {
                        std::lock_guard<std::mutex> lock(exceptionMutex);
                        if (!exception) {
                            exception = std::current_exception();
                        }
                    }
                    done.count_down();
                });
            }

            while (!done.try_wait()) {
                if (TaskType task; tryPopTask(task)) {
                    runAsSearcher(task);
                } else {
                    std::this_thread::yield();
                }
            }

            searching_.fetch_sub(1);

            // The caller no longer takes tasks; pending tasks of other submitters
            // may now require a sleeping worker.
            wakeIfNeeded();

            if (exception) {
                std::rethrow_exception(exception);
            }
        }


        /**
         * @brief Executes one pending task on the calling thread, or yields if none is available.
         *
         * The calling thread is not counted as searching; within the pool, runAndWait()
         * uses its own loop for this purpose.
         */
        void runPendingTask() {
            if (TaskType task; tryPopTask(task)) {
                pendingTasks_.fetch_sub(1);
                wakeIfNeeded();
                task();
            } else {
                std::this_thread::yield();
            }
        }
    };



}
