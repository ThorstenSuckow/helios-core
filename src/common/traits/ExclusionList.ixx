/**
 * @file ExclusionList.ixx
 * @brief Trait for ExclusionList / computing A \ B.
 */
module;

#include <type_traits>
export module helios.core.common.traits:ExclusionList;

import helios.core.common.types;
import :IsInList;
import :UniqueList;

export namespace helios::core::common::traits {

    template<typename ... TLists>
    struct ExclusionList;

    template<typename ...TList>
    struct ExclusionList<types::TypeList<>, types::TypeList<TList...>> {
        using list = types::TypeList<>;
    };

    template<typename THead, typename ...TList,  typename ...TRest>
    struct ExclusionList<types::TypeList<THead, TList...>, types::TypeList<TRest...>> {

    private:

        using Tail = ExclusionList<types::TypeList<TList...>, types::TypeList<TRest...>>::list;

    public:

        using list = std::conditional_t<
            IsInList<THead, types::TypeList<TRest...>>::value,
            Tail,
            typename UniqueList<
                types::TypeList<>,
                typename Tail::template Prepend<THead>
            >::list
        >;

    };


}