module;

#include <concepts>

export module helios.core.common.traits:ConcatList;

import helios.core.common.types;

export namespace helios::core::common::traits {

    template<typename ... TLists>
    struct ConcatList;

    template<typename ... TFirst, typename ... TSecond>
    struct ConcatList<types::TypeList<TFirst...>, types::TypeList<TSecond...>> {

        using list = types::TypeList<TFirst..., TSecond...>;

    };

}