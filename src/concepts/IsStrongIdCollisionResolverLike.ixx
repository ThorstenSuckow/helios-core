/**
 * @file IsStrongIdCollisionResolverLike.ixx
 * @brief Concept for strong ID collision detection strategies.
 */
module;

#include <concepts>

export module helios.core.concepts:IsStrongIdCollisionResolverLike;

import helios.core.common.types;

using namespace helios::core::common::types;
export namespace helios::core::concepts {

    /**
     * @brief Constraint for types that can track strong ID uniqueness.
     *
     * @tparam T The lookup strategy type to check.
     */
    template <typename T>
    concept IsStrongIdCollisionResolverLike = requires(T& t, const StrongId_t id)
    {
        {t.add(id)}->std::same_as<bool>;
        {t.remove(id)}->std::same_as<bool>;
        {t.has(id)}->std::same_as<bool>;
    };

}