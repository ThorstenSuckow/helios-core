module;

#include <vector>
#include <thread>

export module helios.core.thread.ThreadPool:JoinThreads;

export namespace helios::core::thread {


    /**
     * @brief Thread-collection that makes sure join() is called upon destruction.
     *
     * @see Williams, Anthony: Concurrency in Action, 2nd; Manning Publications [p. 275]
     */
    class JoinThreads {

        std::vector<std::thread>& threads;

    public:
        explicit JoinThreads(std::vector<std::thread>& threads) : threads(threads) {}

        ~JoinThreads() {
            for (auto& thread : threads) {
                if (thread.joinable()) {
                    thread.join();
                }
            }
        }

    };


}