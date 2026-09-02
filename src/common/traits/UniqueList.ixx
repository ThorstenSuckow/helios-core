module;

#include <concepts>

export module helios.core.common.traits:UniqueList;

import helios.core.common.types;

export namespace helios::core::common::traits {
    
    
/**
 * @brief Primary template for Contains; membership test is available with its partial specialization.
 * @tparam TType The handle to check for.
 * @tparam TTail The list of handles to check within.
 */
template <typename TType, typename TTail>
struct Contains {};

/**
 * @brief Partial specialization of Contains for checking if TType appears in List<TTail...>.
 * @tparam TType The handle for comparison.
 * @tparam TTail The element types contained in List.
 */
template <typename TType, typename... TTail>
struct Contains<TType, types::TypeList<TTail...>> {
    static constexpr bool value = (std::is_same_v<TType, TTail> || ...);
};

/**
 * @brief Primary template for AppendIfUnique.
 * @tparam TType The handle for comparison.
 * @tparam TList The list to check for duplicates.
 * @tparam exists true if TType already exists in TList, false otherwise.
 */
template <typename TType, typename TList, bool exists = Contains<TType, TList>::value>
struct AppendIfUnique {};

/**
 * @brief Partial specialization of AppendIfUnique for when TType already exists in TTail.
 * Defines `list` as List w/o repeated TType.
 * @tparam TType The handle for comparison.
 * @tparam TTail The element types contained in List.
 */
template <typename TType, typename... TTail>
struct AppendIfUnique<TType, types::TypeList<TTail...>, true> {
    using list = types::TypeList<TTail...>;
};

/**
 * @brief Partial specialization for AppendIfUnique for when TType does not appear in TTail.
 * Defines list as a list that contains TType and the remaining elememts.
 * @tparam TType The handle for comparison.
 * @tparam TTail The element types contained in List.
 */
template <typename TType, typename... TTail>
struct AppendIfUnique<TType, types::TypeList<TTail...>, false> {
    using list = types::TypeList<TTail..., TType>;
};

/**
 * @brief Primary template for deducing a list of unique handles.
 * @tparam TUniqueList The resulting unique list (accumulator).
 * @tparam TInputList The input list that may contains multiple equal handle types.
 */
template <typename TUniqueList, typename TInputList>
struct UniqueList {};

/**
 * @brief Partial specialization for empty List<>, in which case the elements for TUniqueList are used for list.
 * @tparam TUniqueList The unique list of handles
 */
template <typename... TUniqueList>
struct UniqueList<types::TypeList<TUniqueList...>, types::TypeList<>> {
    using list = types::TypeList<TUniqueList...>;
};

/**
 * @brief Partial specialization for UniqueList that recursively build list membertypedef
 * out of List with at least one entry.
 *
 * @tparam TUniqueList The resulting unique handle list.
 * @tparam THead First entry of input list.
 * @tparam TTail The remaining elements of the List.
 */
template <typename... TUniqueList, typename THead, typename... TTail>
struct UniqueList<types::TypeList<TUniqueList...>, types::TypeList<THead, TTail...>> {
    using list = UniqueList<
        typename AppendIfUnique<THead, types::TypeList<TUniqueList...>>::list,
        types::TypeList<TTail...>
    >::list;
};
    
    
}