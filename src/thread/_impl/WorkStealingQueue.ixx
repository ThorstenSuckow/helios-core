module;

#include <deque>
#include <mutex>


export module helios.core.thread.ThreadPool:WorkStealingQueue;

import :FunctionWrapper;

export namespace helios::core::thread {

    /**
     *
     * @see Williams, Anthony: Concurrency in Action, 2nd; Manning Publications [p. 312]
     */
    class WorkStealingQueue {

    private:
        using DataType =  FunctionWrapper;

        std::deque<DataType> queue_;
        mutable std::mutex mutex_;


    public:

        WorkStealingQueue() = default;

        WorkStealingQueue(const WorkStealingQueue&) = delete;
        WorkStealingQueue& operator=(const WorkStealingQueue& other) = delete;

        void push(DataType&& data) {
            std::lock_guard<std::mutex> lock(mutex_);

            queue_.push_front(std::move(data));
        }

        bool empty() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return queue_.empty();
        }

        bool tryPop(DataType& data) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (queue_.empty()) {
                return false;
            }
            data = std::move(queue_.front());
            queue_.pop_front();
            return true;
        }

        bool trySteal(DataType& data) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (queue_.empty()) {
                return false;
            }
            data = std::move(queue_.back());
            queue_.pop_back();
            return true;
        }


    };



}
