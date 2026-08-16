/**
 * @file ReadWriteDoubleBufferFwd.ixx
 * @brief RFwd declaration for ReadWriteDoubleBufferFwd.
 */
module;

#include <span>
#include <vector>
#include <cstddef>

export module helios.core.container:ReadWriteDoubleBufferFwd;

export namespace helios::core::container {
    template <typename T>
    class ReadWriteDoubleBuffer;
}