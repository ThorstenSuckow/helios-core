/**
 * @file TypeId.ixx
 * @brief Unique type identifier.
 */
module;

#include <cstddef>
#include <functional>

export module helios.core.common.types:TypeId;

import :TypeDefs;
import :TypeIndexer;

export namespace helios::core::common::types {

/**
 * @brief Unique type identifier per arbitrary domain.
 */
template <typename TDomain>
class TypeId {

    TypeId_t id_{0};

public:
    using DomainType = TDomain;

    explicit TypeId(const TypeId_t id) : id_(id) {}

    explicit TypeId(types::no_init_t /*unused*/) {}

    [[nodiscard]] TypeId_t value() const noexcept {
        return id_;
    }

    template <typename T>
    [[nodiscard]] static TypeId id() {
        static const TypeId_t tid = TypeIndexer<TDomain>::template typeIndex<T>();
        return TypeId(tid);
    }

    friend constexpr bool operator==(TypeId, TypeId) noexcept = default;
};

} // namespace helios::core::common::types

/**
 * @brief Hash specialization for TypeId.
 */
template <typename TDomain>
struct std::hash<helios::core::common::types::TypeId<TDomain>> {
    std::size_t operator()(const helios::core::common::types::TypeId<TDomain>& id) const noexcept {
        return id.value();
    }
};