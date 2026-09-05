/**
 * @file IntersectionList.ixx
 * @brief Trait for IntersectionList / computing A \cap B.
 */
module;

#include <type_traits>

export module helios.core.common.traits:IntersectionList;

import helios.core.common.types;

import :UniqueList;

export namespace helios::core::common::traits {


    template<typename ... TLists>
    struct IntersectionList;

    template<typename ... TRest>
    struct IntersectionList<types::TypeList<>, types::TypeList<TRest...>> {
        using list = types::TypeList<>;
    };

    template <typename THead, typename ... TLeft, typename ... TRight>
    struct IntersectionList<
        types::TypeList<THead, TLeft...>,
        types::TypeList<TRight...>> {

    private:
        using Tail = IntersectionList<
            types::TypeList<TLeft...>, types::TypeList<TRight...>
        >::list;

    public:
        using list = std::conditional_t<
            IsInList<THead, types::TypeList<TRight...>>::value,
            typename UniqueList<
                types::TypeList<>,
                typename Tail::template Prepend<THead>
            >::list,
            Tail
        >;
    };


}