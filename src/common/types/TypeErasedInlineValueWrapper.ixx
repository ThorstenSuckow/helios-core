/**
 * @file TypeErasedInlineValueWrapper.ixx
 * @brief Concept model implementation with inline storage to prevent heap allocation.
 */

module;

#include <memory>
#include <concepts>
#include<type_traits>
#include <utility>
#include <cstddef>
#include <cassert>
#include <exception>

export module helios.core.common.types:TypeErasedInlineValueWrapper;


export namespace helios::core::common::types {

    template<typename TTypeIdProvider, std::size_t TStorageSize = 64>
    class TypeErasedInlineValueWrapper {

        static constexpr std::size_t StorageSize = TStorageSize;
        static constexpr std::size_t StorageAlignment = alignof(std::max_align_t);

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

            explicit Model(TConcrete&& concreteData) : concreteData_(std::move(concreteData)) {}
            

            [[nodiscard]] void* get()  noexcept override {
                return &concreteData_;
            }

            [[nodiscard]] const void* get() const  noexcept override {
                return &concreteData_;
            }

        };

        alignas(StorageAlignment) std::byte storage_[StorageSize];
        Concept* model_ = nullptr;

        using DestroyFn = void (*)(Concept*) noexcept;
        using MoveFunction = Concept* (*)(Concept*, void*) noexcept;

        DestroyFn destroy_{};
        MoveFunction move_{};

        void destroy() noexcept {
            if (!model_) {
                return;
            }
            destroy_(model_);
            model_ = nullptr;
            destroy_ = nullptr;
            move_ = nullptr;
        }

        void moveFrom (TypeErasedInlineValueWrapper& other) noexcept {
            if (!other.model_) {
                return;
            }

            destroy_ = other.destroy_;
            move_ = other.move_;
            model_ = move_(other.model_, storage_);
            other.destroy_ = nullptr;
            other.move_ = nullptr;
            other.model_ = nullptr;
        }

        TTypeIdProvider typeId_;

    public:

        TypeErasedInlineValueWrapper(const TypeErasedInlineValueWrapper&) = delete;
        TypeErasedInlineValueWrapper& operator=(const TypeErasedInlineValueWrapper&) = delete;
        TypeErasedInlineValueWrapper(TypeErasedInlineValueWrapper&& other) noexcept :
        typeId_(other.typeId_) {moveFrom(other);};
        TypeErasedInlineValueWrapper& operator=(TypeErasedInlineValueWrapper&& other) noexcept {

            if (this != &other) {
                destroy();
                typeId_ = other.typeId_;
                moveFrom(other);
            }
            return *this;
        };

        template<typename TConcrete>
        requires (!std::same_as<std::remove_cvref_t<TConcrete>, TypeErasedInlineValueWrapper>)
        && std::is_rvalue_reference_v<TConcrete&&>
        && (!std::is_const_v<std::remove_reference_t<TConcrete>>)
        explicit TypeErasedInlineValueWrapper(TConcrete&& concrete) noexcept
        :
        typeId_(TTypeIdProvider::template id<std::remove_cvref_t<std::remove_cvref_t<TConcrete>>>())
        {

            using Type = std::remove_cvref_t<TConcrete>;
            using ModelType = Model<Type>;




            static_assert(sizeof(ModelType) <= StorageSize,
                "TypeErasedInlineValueWrapper cannot wrap types larger than the inline storage.");
            static_assert(alignof(ModelType) <= StorageAlignment,
                "TypeErasedInlineValueWrapper cannot wrap types with larger alignment than the inline storage.");

            static_assert(std::is_nothrow_move_constructible_v<Type>,
                "TypeErasedInlineValueWrapper requires types to be nothrow move constructible.");

            model_ = std::construct_at(reinterpret_cast<ModelType*>(storage_), std::move(concrete));

            destroy_ = [](Concept* model) noexcept {
                std::destroy_at(static_cast<ModelType*>(model));
            };

            move_ = [](Concept* source, void* destination) noexcept -> Concept* {
                auto* sourceModel = static_cast<ModelType*>(source);
                auto* targetModel = std::construct_at(static_cast<ModelType*>(destination), std::move(sourceModel->concreteData_));
                std::destroy_at(sourceModel);
                return targetModel;
            };
        }

        ~TypeErasedInlineValueWrapper() noexcept {
            destroy();
        }

        template<typename TConcrete>
        [[nodiscard]] TConcrete& get() noexcept {

            using ConcreteType = std::remove_cvref_t<TConcrete>;

            if (typeId_ != TTypeIdProvider::template id<ConcreteType>()) [[unlikely]] {
                assert(false && "TypeErasedInlineValueWrapper does not contain the requested concrete type.");
                std::terminate();
            }

            return *static_cast<ConcreteType*>(model_->get());
        }

        template<typename TConcrete>
       [[nodiscard]] const TConcrete& get() const noexcept {

            using ConcreteType = std::remove_cvref_t<TConcrete>;

            if (typeId_ != TTypeIdProvider::template id<ConcreteType>()) [[unlikely]] {
                assert(false && "TypeErasedInlineValueWrapper does not contain the requested concrete type.");
                std::terminate();
            }

            const Concept* model = model_;
            return *static_cast<const ConcreteType*>(model->get());
        }

    };


}