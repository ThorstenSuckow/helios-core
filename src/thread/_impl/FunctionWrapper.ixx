module;

#include <memory>


export module helios.core.thread.ThreadPool:FunctionWrapper;

export namespace helios::core::thread {

    /**
     * @brief Runtime-concept idiom for functions.
     *
     * @see Williams, Anthony: Concurrency in Action, 2nd; Manning Publications [p. 305]
     */
    class FunctionWrapper {

        struct ImplBase {
            virtual void call() = 0;
            virtual ~ImplBase() = default;
        };

        std::unique_ptr<ImplBase> impl_;

        template<typename F>
        struct Impl : ImplBase {
            F f;
            explicit Impl(F&& f_) : f(std::move(f_)) {}
            void call() override {
                f();
            }
        };

    public:

        FunctionWrapper() = default;

        FunctionWrapper(FunctionWrapper&&  other)
        :
        impl_(std::move(other.impl_))
        {}

        FunctionWrapper(const FunctionWrapper&) = delete;
        FunctionWrapper(FunctionWrapper&) = delete;


        template<typename F>
        explicit FunctionWrapper(F&& f)
        :
        impl_(std::make_unique<Impl<F>>(std::move(f)))
        {}

        void operator()() {
            impl_->call();
        }

        FunctionWrapper& operator=(FunctionWrapper&& other) noexcept {
            impl_ = std::move(other.impl_);
            return *this;
        }

        FunctionWrapper& operator=(const FunctionWrapper&& other) = delete;

    };



}
