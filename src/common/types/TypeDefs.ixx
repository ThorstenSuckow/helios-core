/**
 * @file TypeDefs.ixx
 * @brief Core type definitions and tag types for the helios engine.
 */
module;

#include <cstdint>

export module helios.core.common.types:TypeDefs;

export namespace helios::core::common::types {

/**
 * @brief Underlying integer type for all strong identifiers.
 */
using StrongId_t = uint32_t;

using TypeId_t = uint32_t;

/**
 * @brief Tag type used to indicate skipping default initialization.
 */
struct no_init_t {};

/**
 * @brief Tag constant for requesting uninitialized construction.
 */
inline constexpr no_init_t no_init;

} // namespace helios::core::common::types