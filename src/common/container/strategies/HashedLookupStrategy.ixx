/**
 * @file HashedLookupStrategy.ixx
 * @brief Hash-based lookup strategy for strong ID collision detection.
 */
module;

#include <cstddef>
#include <unordered_set>

export module helios.core.common.container.strategies:HashedLookupStrategy;

import helios.core.common.types;

using namespace helios::core::common::types;
export namespace helios::core::common::container::strategies {

/**
 * @brief Lookup strategy using an unordered set for O(1) amortized lookups.
 *
 * `HashedLookupStrategy` maintains a set of registered strong IDs
 * and provides constant-time insertion, removal, and membership tests.
 * Used as the default lookup strategy in `EntityRegistry`.
 *
 * @see EntityRegistry
 * @see LinearLookupStrategy
 */
template <typename TDomain, typename TData = StrongId_t>
class HashedLookupStrategy {

    /**
     * @brief Set of registered datas.
     */
    std::unordered_set<TData> strongIds_;

public:
    explicit HashedLookupStrategy() = default;

    /**
     * @brief Constructs a strategy with pre-allocated capacity.
     *
     * @param capacity The initial bucket count to reserve.
     */
    explicit HashedLookupStrategy(const size_t capacity) {
        strongIds_.reserve(capacity);
    };

    /**
     * @brief Registers a data.
     *
     * @param id The data to add.
     *
     * @return True if inserted, false if already present.
     */
    [[nodiscard]] bool add(const TData id) {
        return strongIds_.insert(id).second;
    }

    /**
     * @brief Removes a data.
     *
     * @param id The data to remove.
     *
     * @return True if removed, false if not found.
     */
    [[nodiscard]] bool remove(const TData id) {
        return strongIds_.erase(id) > 0;
    }

    /**
     * @brief Checks whether a data is registered.
     *
     * @param id The data to test.
     *
     * @return True if the ID is present.
     */
    [[nodiscard]] bool has(const TData id) const {
        return strongIds_.contains(id);
    }
};

} // namespace helios::core::common::container::strategies