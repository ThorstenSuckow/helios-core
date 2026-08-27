/**
 * @file helios::core::common::TypeIndexer.ixx
 * @brief Utility for generating unique type indices at runtime.
 */
module;

#include <atomic>
#include <cstddef>


export module helios.core.common.types:TypeIndexer;

import :TypeDefs;

export namespace helios::core::common::types {

    /**
     * @class TypeIndexer
     * @brief Utility class to generate unique type indices at runtime.
     *
     * @tparam Group A tag type used to create separate index spaces.
     */
    template<typename Group>
    class TypeIndexer {

        /**
         * @brief A static atomic counter used for generating unique type indices.
         */
        static inline std::atomic<TypeId_t> counter_{0};

    public:

        /**
         * @brief Generates and returns a unique type index for a specific type.
         *
         * @tparam T The type for which to retrieve the unique index.
         *
         * @return A unique index representing the specific type T within this Group.
         */
        template<typename T>
        static TypeId_t typeIndex() {
            static const TypeId_t typeIndex = counter_.fetch_add(1, std::memory_order::relaxed);
            return typeIndex;
        }

    };
}