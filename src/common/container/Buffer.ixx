/**
 * @file Buffer.ixx
 * @brief Abstract base class for buffer implementations.
 */
module;

#include <memory>
#include <span>
#include <vector>

export module helios.core.common.container:Buffer;


export namespace helios::core::common::container {

    /**
     * @brief Abstract base class for type-erased buffer implementations.
     *
     * @details Provides a minimal interface for buffer operations without
     * exposing the concrete element type. Used as a base for heterogeneous
     * buffer storage in type-indexed containers.
     */
    class Buffer {

    public:

        /**
         * @brief Default constructor.
         */
        Buffer() = default;

        /**
         * @brief Virtual destructor for proper polymorphic cleanup.
         */
        virtual ~Buffer() = default;

        /**
         * @brief Clears all elements from the buffer.
         */
        virtual void clear() = 0;

    };
}
