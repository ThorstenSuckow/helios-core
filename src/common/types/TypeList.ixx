module;

#include <cstddef>

export module helios.core.common.types:TypeList;


export namespace helios::core::common::types {

    /**
     * @brief TypeList is a compile-time list of types.
     * @tparam TTypes The types contained in the list.
     */
    template <typename... TTypes>
    struct TypeList {

        static constexpr std::size_t size = sizeof...(TTypes);
        template<typename T>
        using Prepend = TypeList<T, TTypes...>;
    };

} // namespace helios::core::common::types