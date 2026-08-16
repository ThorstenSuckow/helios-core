module;

#include <memory>
#include <vector>
#include <cassert>
#include <exception>

export module helios.core.container:TypeMap;

import helios.core.common;

export namespace helios::core::container {

    /**
     * @brief Type-indexed container for owning a collection of type-erased objects across arbitrary domains.
     */
    template<typename TDomain>
    class TypeMap {

        using TypeMapItemTypeId = common::TypeId<TDomain>;

        class Concept {
        public:
            virtual ~Concept() = default;


        };

        template<typename TType>
        class Model final : public Concept {

            TType type_;

        public:
            template<typename... Args>
            explicit Model(Args&& ... args) : type_(std::forward<Args>(args)...) {}

            TType& type() {
                return type_;
            }
        };


        std::vector<std::unique_ptr<Concept>> typeMap_;

    public:

        explicit TypeMap() = default;

        template<typename TType, typename... Args>
        TType&  add(Args&& ...args) {
            auto typeId = TypeMapItemTypeId::template id<TType>();
            auto idx = typeId.value();

            auto model = std::make_unique<Model<TType>>(std::forward<Args>(args)...);

            if (typeMap_.size() <= idx) {
                typeMap_.resize(idx + 1);
            }

            if (typeMap_[idx]) [[unlikely]] {
                assert(false && "Resource already registered.");
                std::terminate();
            }

            typeMap_[idx] = std::move(model);

            return (static_cast<Model<TType>&>(*typeMap_[idx])).type();
        }

        template<typename TType>
        TType& get() {

            auto typeId = TypeMapItemTypeId::template id<TType>();
            auto idx = typeId.value();

            if (idx >= typeMap_.size() || !typeMap_[idx]) [[unlikely]] {
                assert(false && "Resource not found.");
                std::terminate();
            }

            return (static_cast<Model<TType>&>(*typeMap_[idx])).type();
        }

        template<typename TType>
        [[nodiscard]] TType* tryGet() {
            auto typeId = TypeMapItemTypeId::template id<TType>();
            auto idx = typeId.value();

            if (idx >= typeMap_.size() || !typeMap_[idx]) {
                return nullptr;
            }

            return &(static_cast<Model<TType>&>(*typeMap_[idx]).type());
        }

        template<typename TType, typename ... TArgs>
        [[nodiscard]] TType& getOrEmplace(TArgs&&... args) {

            if (auto* inst = tryGet<TType>()) {
                return *inst;
            }
            return add<TType>(std::forward<TArgs>(args)...);
        }

        bool clear() {
            typeMap_.clear();
            return true;
        }


    };


}