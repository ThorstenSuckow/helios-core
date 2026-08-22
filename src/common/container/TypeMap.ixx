module;

#include <memory>
#include <vector>
#include <cassert>
#include <exception>
#include <utility>

export module helios.core.common.container:TypeMap;

import helios.core.common.types;

export namespace helios::core::common::container {

    /**
     * @brief Type-indexed container for owning a collection of type-erased objects across arbitrary domains.
     */
    template<typename TDomain>
    class TypeMap {

        using TypeMapItemTypeId = common::types::TypeId<TDomain>;

        class Concept {
        public:
            virtual ~Concept() = default;
        };

        template<typename TType>
        class OwnedModel final : public Concept {
            TType type_;

        public:
            template<typename... Args>
            explicit OwnedModel(Args&& ... args) : type_(std::forward<Args>(args)...) {}

            TType& type() {
                return type_;
            }

            TType& type() const {
                return type_;
            }
        };


        std::vector<std::unique_ptr<Concept>> owned_;

        std::vector<TypeMap*> borrowedMaps_;

        bool containsMap(TypeMap* typeMap) const noexcept {
            if (this == typeMap) {
                return true;
            }

            for (const auto* borrowedMap : borrowedMaps_) {
                if (borrowedMap->containsMap(typeMap)) {
                    return true;
                }
            }
            return false;
        }

        bool clearable_ = false;
    public:

        explicit TypeMap() = default;

        explicit TypeMap(bool clearable) : clearable_(clearable) {}


        TypeMap& borrow(TypeMap& other) {

            if (other.containsMap(this)) [[unlikely]] {
                assert(false && "Circular TypeMap borrowing detected.");
                std::terminate();
            }

            borrowedMaps_.push_back(&other);

            return *this;
        }
        
        
        template<typename TType, typename... Args>
        TType& emplace(Args&& ...args) {

            auto typeId = TypeMapItemTypeId::template id<TType>();
            auto idx = typeId.value();

            auto model = std::make_unique<OwnedModel<TType>>(std::forward<Args>(args)...);

            if (owned_.size() <= idx) {
                owned_.resize(idx + 1);
            }

            if (owned_[idx]) [[unlikely]] {
                assert(false && "Resource already registered.");
                std::terminate();
            }

            owned_[idx] = std::move(model);

            return (static_cast<OwnedModel<TType>&>(*owned_[idx])).type();
        }

        template<typename TType>
        TType& get() {
            return const_cast<TType&>(std::as_const(*this).template get<TType>());
        }

        template<typename TType>
        const TType& get() const {

            auto* inst = tryGet<TType>();
            if (inst) {
                return *inst;
            }

            assert(false && "Resource not found.");
            std::terminate();

        }

        template<typename TType>
        [[nodiscard]] TType* tryGet() {
            return const_cast<TType*>(std::as_const(*this).template tryGet<TType>());
        }

        template<typename TType>
        [[nodiscard]] TType* tryGet() const {

            using Type = std::remove_cvref_t<TType>;
            auto typeId = TypeMapItemTypeId::template id<Type>();
            auto idx = typeId.value();

            if (idx < owned_.size() && owned_[idx]) {
                return &(static_cast<OwnedModel<Type>&>(*owned_[idx]).type());
            }

            for (auto* borrowedMap : borrowedMaps_) {
                if (auto* inst = borrowedMap->template tryGet<TType>()) {
                    return inst;
                }
            }

            return nullptr;
        }

        template<typename TType, typename ... TArgs>
        [[nodiscard]] TType& getOrEmplace(TArgs&&... args) {

            if (auto* inst = tryGet<TType>()) {
                return *inst;
            }
            return emplace<TType>(std::forward<TArgs>(args)...);
        }

        bool clear() {
            if (clearable_) {
                owned_.clear();
            }
            return clearable_;
        }


    };


}