/**
 * @file algorithms.ixx
 * @brief Core algorithms and hash functions for the helios engine.
 */
module;

#include <string>

export module helios.core.common.types:FuncDefs;



export namespace helios::core::common::types {

    /**
     * @brief Computes a 32-bit FNV-1a hash from a string.
     *
     * @param str The string view to hash.
     *
     * @return 32-bit hash value.
     *
     * @note This is not a cryptographic hash. Do not use for security purposes.
     *
     * @see https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
     */
    [[nodiscard]] constexpr uint32_t fnv1a_hash(const std::string_view str) noexcept {
        uint32_t hash = 2166136261U;
        for (char c: str) {
            hash ^= static_cast<uint8_t>(c);
            hash *= 16777619U;
        }
        return hash;
    }


}