module;

#include <concepts>

export module helios.core.common.traits:ConcatList;

import helios.core.common.types;

export namespace helios::core::common::traits {

    template<typename ... TLists>
    struct ConcatList;

    template<>
    struct ConcatList<> {
        using list = types::TypeList<>;
    };


    template<typename... TElements>
    struct ConcatList<types::TypeList<TElements...>> {
        using list = types::TypeList<TElements...>;
    };

    template<typename ... TLeft, typename ... TRight, typename ... TRest>
    struct ConcatList<types::TypeList<TLeft...>, types::TypeList<TRight...>, TRest...> {

        using list = ConcatList<types::TypeList<TLeft..., TRight...>, TRest...>::list;

    };
}