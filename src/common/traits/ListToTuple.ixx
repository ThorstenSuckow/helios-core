module;

#include <tuple>

export module helios.core.common.traits:ListToTuple;

import helios.core.common.types;

export namespace helios::core::common::traits {

    /**
* @brief Primary template to list/tuple conversion.
 * @tparam TList
 */
    template <typename TList>
    struct ListToTuple {};

    /**
     * @brief Partial specialization of ToTuple for HandleList.
     * @tparam TElements The element types contained in HandleList.
     */
    template <typename... TElements>
    struct ListToTuple<types::TypeList<TElements...>> {
        using tuple = std::tuple<TElements...>;
    };

}
