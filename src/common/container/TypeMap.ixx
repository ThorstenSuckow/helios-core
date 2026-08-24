/**
 * @file TypeMap.ixx
 * @brief TypeMap for maintaining a map of type indexed data.
 */
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
     *
     * Allows for borrowing other type maps for inspecting other TypeMap's data.
     * Data can be bound by references or completely owned by the TypeMap.
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

            const TType& type() const {
                return type_;
            }
        };

        template<typename TType>
        class BoundRefModel final : public Concept {
            TType* type_;

        public:

            explicit BoundRefModel(TType& type) : type_(&type) {
                assert(type_ != nullptr && "Unexpected nullptr.");
            }

            TType& type() {
                assert(type_ != nullptr && "Unexpected nullptr.");
                return *type_;
            }

            const TType& type() const {
                assert(type_ != nullptr && "Unexpected nullptr.");
                return *type_;
            }
        };


        std::vector<std::unique_ptr<Concept>> ownedModels_;

        std::vector<std::unique_ptr<Concept>> boundRefs_;

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
        
        template<typename TType>
        TType& bind(TType& type) {

            using Type = std::remove_cvref_t<TType>;

            auto typeId = TypeMapItemTypeId::template id<Type>();
            auto idx = typeId.value();

            auto model = std::make_unique<BoundRefModel<Type>>(type);

            if (boundRefs_.size() <= idx) {
                boundRefs_.resize(idx + 1);
            }

            if (boundRefs_[idx]) [[unlikely]] {
                assert(false && "Resource already registered.");
                std::terminate();
            }

            boundRefs_[idx] = std::move(model);

            return (static_cast<BoundRefModel<Type>&>(*boundRefs_[idx])).type();

        }


        template<typename TType, typename... Args>
        TType& emplace(Args&& ...args) {
            return emplace<TType>(TType{std::forward<Args>(args)...});
        }

        template<typename TType>
        requires (!std::is_lvalue_reference_v<TType>)
        auto& emplace(TType&& typeToEmplace) {

            using Type = std::remove_cvref_t<TType>;
            auto typeId = TypeMapItemTypeId::template id<Type>();
            auto idx = typeId.value();

            auto model = std::make_unique<OwnedModel<Type>>(std::move(typeToEmplace));

            if (ownedModels_.size() <= idx) {
                ownedModels_.resize(idx + 1);
            }

            if (ownedModels_[idx]) [[unlikely]] {
                assert(false && "Resource already registered.");
                std::terminate();
            }

            ownedModels_[idx] = std::move(model);

            return (static_cast<OwnedModel<Type>&>(*ownedModels_[idx])).type();
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

            if (idx < ownedModels_.size() && ownedModels_[idx]) {
                return &(static_cast<OwnedModel<Type>&>(*ownedModels_[idx]).type());
            } 
            
            if (idx < boundRefs_.size() && boundRefs_[idx]) {
                return &(static_cast<BoundRefModel<Type>&>(*boundRefs_[idx]).type());
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

        /**
         * @brief Clears **this** TypeMap's owned data.
         *
         * @return
         */
        bool clearOwned() {
            if (clearable_) {
                ownedModels_.clear();
            }
            return clearable_;
        }

        /**
         * @brief Clears **this** TypeMap's bound data.
         *+
         * @return
         */
        bool clearBound() {
            if (clearable_) {
                boundRefs_.clear();
            }
            return clearable_;
        }


    };


}