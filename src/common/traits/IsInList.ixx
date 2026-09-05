module;

#include <concepts>
#include <cstddef>
#include <type_traits>

export module helios.core.common.traits:IsInList;

import helios.core.common.types;

export namespace helios::core::common::traits {


template <typename TNeedle, typename... THaystack>
struct IsInList;

template <typename TNeedle>
struct IsInList<TNeedle> {
    constexpr static bool value = false;
    constexpr static std::size_t index = 0;
};

template <typename TNeedle, typename First, typename... Rest>
struct IsInList<TNeedle, First, Rest...> {
    constexpr static bool value = std::same_as<TNeedle, First>
        || IsInList<TNeedle, Rest...>::value;

    constexpr static std::size_t index = std::same_as<TNeedle, First>
        ? 0
        : 1 + IsInList<TNeedle, Rest...>::index;
};

template <typename TNeedle, typename First, typename... Rest>
struct IsInList<TNeedle, types::TypeList<First, Rest...>> {

    constexpr static bool value = std::same_as<TNeedle, First>
        || IsInList<TNeedle, types::TypeList<Rest...>>::value;

    constexpr static std::size_t index = std::same_as<TNeedle, First>
        ? 0
        : 1 + IsInList<TNeedle, types::TypeList<Rest...>>::index;
};



template <typename TLeft, typename TRight>
struct IsSubset;

template <typename ... TLftItems, typename ... TRgtItems>
struct IsSubset<types::TypeList<TLftItems...>, types::TypeList<TRgtItems...>> {
    constexpr static bool value = (IsInList<TLftItems, TRgtItems...>::value && ...);
};


}
