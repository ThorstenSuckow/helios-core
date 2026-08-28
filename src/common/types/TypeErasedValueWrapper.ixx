/**
 * @file TypeErasedValueWrapper.ixx
 * @brief Concept model implementation with heap allocation.
 */
module;

#include <memory>


export module helios.core.common.types:TypeErasedValueWrapper;


export namespace helios::core::common::types {

    class TypeErasedValueWrapper {

        class Concept {

        public:
            virtual ~Concept() = default;

            [[nodiscard]] virtual void* get()  noexcept = 0;

            [[nodiscard]] virtual const void* get() const  noexcept = 0;
        };


        template<typename TConcrete>
        class Model final : public Concept {

        public:

            TConcrete concreteData_;

            Model(TConcrete concreteData) : concreteData_(std::move(concreteData)) {}
            

            [[nodiscard]] void* get()  noexcept override {
                return &concreteData_;
            }

            [[nodiscard]] const void* get() const  noexcept override {
                return &concreteData_;
            }

        };


        std::unique_ptr<Concept> model_;

    public:

        TypeErasedValueWrapper(const TypeErasedValueWrapper&) = delete;
        TypeErasedValueWrapper& operator=(const TypeErasedValueWrapper&) = delete;
        TypeErasedValueWrapper(TypeErasedValueWrapper&&) = default;
        TypeErasedValueWrapper& operator=(TypeErasedValueWrapper&&) = default;

        template<typename TConcrete>
        requires(!std::same_as<std::remove_cvref_t<TConcrete>, TypeErasedValueWrapper>)
        explicit TypeErasedValueWrapper(TConcrete&& concrete)
        : model_(std::make_unique<Model<std::remove_cvref_t<TConcrete>>>(std::move(concrete))) {}

        template<typename TConcrete>
        [[nodiscard]] TConcrete& get() noexcept {
            return *static_cast<TConcrete*>(model_->get());
        }

        template<typename TConcrete>
       [[nodiscard]] const TConcrete& get() const noexcept {
            return *static_cast<const TConcrete*>(model_->get());
        }

    };


}