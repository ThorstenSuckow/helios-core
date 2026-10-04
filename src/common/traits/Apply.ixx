module;


export module helios.core.common.traits:Apply;

import helios.core.common.types;

export namespace helios::core::common::traits {


    template<typename TList>
    struct Apply;

    template<typename ... TElements>
    struct Apply<types::TypeList<TElements...>> {

        template<typename TFunc>
        static void forEach(TFunc&& func) {
            (func.template operator()<TElements>(), ...);
        }

    };


}