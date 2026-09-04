/**
 * @file WrapElement.ixx
 * @brief Trait for wrapping the elements of a list with another arbitrary type.
 */
module;

#include <concepts>

export module helios.core.common.traits:WrapElements;

import helios.core.common.types;

export namespace helios::core::common::traits {

    template<
        template <typename> typename TWrapper,
        typename TList
    >
    struct WrapElements;

    template<
        template <typename> typename TWrapper,
        typename ... TElements
    >
    struct WrapElements<TWrapper, types::TypeList<TElements...>> {

        using list = types::TypeList<TWrapper<TElements>...>;

    };

}