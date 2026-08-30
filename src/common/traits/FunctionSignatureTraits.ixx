/**
 * @file FunctionSignatureTraits.ixx
 * @brief Traits for exposing function signature properties.
 */
module;

#include <cstddef>
#include <tuple>

export module helios.core.common.traits:FunctionSignatureTraits;

export namespace helios::core::common::traits {

template <typename TResult, typename... TArgs>
struct FunctionSignatureTraitsBase {
    using ReturnType = TResult;
    using ArgumentTypes = std::tuple<TArgs...>;
    using UnqualifiedArgumentTypes = std::tuple<std::remove_cvref_t<TArgs>...>;

    static constexpr size_t NumArgs = sizeof...(TArgs);

    template <std::size_t I>
    using ArgumentType = std::tuple_element_t<I, ArgumentTypes>;

    template <std::size_t I>
    using UnqualifiedArgumentType = std::remove_cvref_t<ArgumentType<I>>;
};

template <typename>
struct FunctionSignatureTraits;

template <typename TResult, typename TClass, typename... TArgs>
struct FunctionSignatureTraits<TResult (TClass::*)(TArgs...) noexcept>
    : FunctionSignatureTraitsBase<TResult, TArgs...> {};

template <typename TResult, typename TClass, typename... TArgs>
struct FunctionSignatureTraits<TResult (TClass::*)(TArgs...) const noexcept>
    : FunctionSignatureTraitsBase<TResult, TArgs...> {};

template <typename TResult, typename TClass, typename... TArgs>
struct FunctionSignatureTraits<TResult (TClass::*)(TArgs...) const> : FunctionSignatureTraitsBase<TResult, TArgs...> {};

} // namespace helios::core::common::traits
