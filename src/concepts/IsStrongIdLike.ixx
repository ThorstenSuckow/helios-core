/**
 * @file IsStrongIdLike.ixx
 * @brief Concept for types modelling a strong identifier.
 */
module;

#include <concepts>

export module helios.core.concepts:IsStrongIdLike;

import helios.core.types;

using namespace helios::core::types;
export namespace helios::core::concepts {

    /**
     * @brief Constraint for types that behave as a strong, validatable identifier.
     *
     * @tparam T The identifier type to check.
     */
    template <typename T>
    concept IsStrongIdLike = requires(T& t)
    {
        {t.isValid()}->std::same_as<bool>;
        {t.value()}->std::same_as<StrongId_t>;
    };

}