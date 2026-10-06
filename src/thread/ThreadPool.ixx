module;

#include <atomic>
#include <deque>
#include <future>
#include <latch>
#include <mutex>
#include <thread>
#include <vector>
#include <type_traits>


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
     * repeatedly invokes runPendingTask() instead of blocking.
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

        ThreadSafeQueue<TaskType> poolWorkQueue_;

        std::vector<std::unique_ptr<WorkStealingQueue>> queues_;

        std::vector<std::thread> threads_;

        JoinThreads joinThreads_;

        inline static thread_local  WorkStealingQueue* localWorkQueue_;
        inline static thread_local  std::size_t localThreadId_;

        void workerThread(const unsigned int threadId) {
            localThreadId_ = threadId;
            localWorkQueue_ = queues_[localThreadId_].get();
            while (running_) {
                runPendingTask();
            }
        }

        bool popTaskFromLocalQueue(TaskType& task) {
            return localWorkQueue_ && localWorkQueue_->tryPop(task);
        }

        bool popTaskFromPoolQueue(TaskType& task) {
            return poolWorkQueue_.tryPop(task);
        }

        bool popTaskFromOtherThreadQueue(TaskType& task) {

            for (unsigned i = 0; i < queues_.size(); ++i) {
                unsigned const threadId = (localThreadId_ + i + 1) % queues_.size();
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

        ThreadPool(const auto numThreads = std::thread::hardware_concurrency())
            : running_(true),
            joinThreads_(threads_) {
            try {
                for (unsigned i = 0; i < numThreads; ++i) {
                    queues_.emplace_back(std::make_unique<WorkStealingQueue>());
                }
                for (unsigned i = 0; i < numThreads; ++i) {
                    threads_.emplace_back(&ThreadPool::workerThread, this, i);
                }
            } catch (...) {
                running_ = false;
                throw;
            }
        }
        ~ThreadPool() {
            running_ = false;
        }


        template<typename FunctionType>
        std::future<typename std::invoke_result_t<FunctionType>> submit(FunctionType f) {
            using ResultType = typename std::invoke_result_t<FunctionType>;

            std::packaged_task<ResultType()> packagedTask(std::move(f));
            std::future<ResultType> future(packagedTask.get_future());

            TaskType task{std::move(packagedTask)};

            if (localWorkQueue_) {
                localWorkQueue_->push(std::move(task));
            } else {
                poolWorkQueue_.push(std::move(task));
            }
            return future;
        }

        template<typename FunctionType>
        void runAndWait(unsigned int numTasks, FunctionType f) {
            std::latch done{numTasks};

            std::mutex exceptionMutex;
            std::exception_ptr exception;

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
                runPendingTask();
            }

            if (exception) {
                std::rethrow_exception(exception);
            }
        }



        void runPendingTask() {
            TaskType task;
            if (popTaskFromLocalQueue(task) ||
                popTaskFromPoolQueue(task) ||
                popTaskFromOtherThreadQueue(task)) {
                task();
            } else {
                std::this_thread::yield();
            }
        }
    };



}
