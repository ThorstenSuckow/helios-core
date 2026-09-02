module;

export module helios.core.common.types:TypeList;


export namespace helios::core::common::types {

    /**
     * @brief TypeList is a compile-time list of types.
     * @tparam TTypes The types contained in the list.
     */
    template <typename... TTypes>
    struct TypeList {


        template<typename T>
        using Prepend = TypeList<T, TTypes...>;
    };

} // namespace helios::core::common::types