module;

#include <queue>
#include <mutex>
#include <memory>
#include <condition_variable>


export module helios.core.thread.ThreadSafeQueue;


export namespace helios::core::thread {

    /**
     *
     * @see Williams, Anthony: Concurrency in Action, 2nd; Manning Publications [pp. 79--80]
     */
    template<typename T>
    class ThreadSafeQueue {

    private:
        mutable std::mutex mutex_;
        std::condition_variable cv_;
        std::queue<T> queue_;

    public:

        ThreadSafeQueue() = default;

        ThreadSafeQueue(const ThreadSafeQueue& other ) {
            std::lock_guard<std::mutex> lock(other.mutex_);
            queue_ = other.queue_;
        } ;


        void push(T value) {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(value));
            cv_.notify_one();
        }

        void waitAndPop(T& value) {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this]{ return !queue_.empty(); });
            value = queue_.front();
            queue_.pop();
        }

        std::shared_ptr<T> waitAndPop() {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this]{ return !queue_.empty(); });
            auto value = std::make_shared<T>(std::move(queue_.front()));
            queue_.pop();
            return value;
        }

        bool tryPop(T& value) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (queue_.empty()) {
                return false;
            }
            value = std::move(queue_.front());
            queue_.pop();
            return true;
        }

        std::shared_ptr<T> tryPop() {

            std::lock_guard<std::mutex> lock(mutex_);
            if (queue_.empty()) {
                return std::shared_ptr<T>();
            }
            auto value = std::make_shared<T>(queue_.front());
            queue_.pop();
            return value;
        }

        [[nodiscard]] bool empty() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return queue_.empty();
        }

    };



}
