/**
 * @file TypeId.ixx
 * @brief Unique type identifier.
 */
module;

#include <functional>
#include <cstddef>

export module helios.core.common:TypeId;


import :TypeIndexer;
import helios.core.common.types;

export namespace helios::core::common {

    /**
     * @brief Unique type identifier per arbitrary domain.
     */
    template<typename TDomain>
    class TypeId {

        size_t id_{0};

    public:

        using DomainType = TDomain;

        explicit TypeId(const size_t id) : id_(id) {}

        explicit TypeId(helios::core::common::types::no_init_t) {}

        [[nodiscard]] size_t value() const noexcept {
            return id_;
        }

        template <typename T>
        [[nodiscard]] static TypeId id() {
            static const size_t tid = helios::core::common::TypeIndexer<TDomain>::template typeIndex<T>();
            return TypeId(tid);
        }

        friend constexpr bool operator==(TypeId, TypeId) noexcept = default;
    };


}


/**
 * @brief Hash specialization for TypeId.
 */
template<typename TDomain>
struct std::hash<helios::core::common::TypeId<TDomain>> {
   std::size_t operator()(const helios::core::common::TypeId<TDomain>& id) const noexcept {
        return id.value();
    }

};