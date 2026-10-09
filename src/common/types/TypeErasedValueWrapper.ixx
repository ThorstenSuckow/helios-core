/**
 * @file TypeErasedValueWrapper.ixx
 * @brief Concept model implementation with heap allocation.
 */
module;

#include <memory>
#include <cassert>
#include <utility>
#include <exception>

export module helios.core.common.types:TypeErasedValueWrapper;

export namespace helios::core::common::types {

template <typename TTypeId>
class TypeErasedValueWrapper {

    class Concept {

    public:
        virtual ~Concept() = default;

        [[nodiscard]] virtual void* underlying() noexcept = 0;

        [[nodiscard]] virtual const void* underlying() const noexcept = 0;
    };

    template <typename TConcrete>
    class Model final : public Concept {

    public:
        TConcrete concreteData_;

        Model(TConcrete concreteData) : concreteData_(std::move(concreteData)) {}

        [[nodiscard]] void* underlying() noexcept override {
            return &concreteData_;
        }

        [[nodiscard]] const void* underlying() const noexcept override {
            return &concreteData_;
        }
    };

    std::unique_ptr<Concept> model_;

    TTypeId typeId_;
public:

    using TypeId = TTypeId;

    TypeErasedValueWrapper(const TypeErasedValueWrapper&) = delete;
    TypeErasedValueWrapper& operator=(const TypeErasedValueWrapper&) = delete;
    TypeErasedValueWrapper(TypeErasedValueWrapper&&) = default;
    TypeErasedValueWrapper& operator=(TypeErasedValueWrapper&&) = default;

    template <typename TConcrete>
        requires(!std::same_as<std::remove_cvref_t<TConcrete>, TypeErasedValueWrapper>)
    explicit TypeErasedValueWrapper(TConcrete&& concrete)
        : model_(std::make_unique<Model<std::remove_cvref_t<TConcrete>>>(std::forward<TConcrete>(concrete))),
          typeId_(TTypeId::template id<std::remove_cvref_t<TConcrete>>()) {}

    TTypeId typeId() const noexcept {
        return typeId_;
    }

    [[nodiscard]] const void* underlying() const noexcept {
        return model_->underlying();
    }

    [[nodiscard]] void* underlying() noexcept {
        return model_->underlying();
    }

    [[nodiscard]] bool hasTypeId(TTypeId typeId) const noexcept {
        return typeId_ == typeId;
    }

    template <typename TConcrete>
    [[nodiscard]] TConcrete& get() noexcept {
        return const_cast<TConcrete&>(
            std::as_const(*this).template get<TConcrete>()
        );
    }

    template <typename TConcrete>
    [[nodiscard]] const TConcrete& get() const noexcept {
        auto expectedTypeId = TTypeId::template id<TConcrete>();

        if (expectedTypeId != typeId_) [[unlikely]] {
            assert(false && "Type mismatch in TypeErasedValueWrapper::get()");
            std::terminate();
        }

        return *static_cast<const TConcrete*>(model_->underlying());
    }
};

} // namespace helios::core::common::types