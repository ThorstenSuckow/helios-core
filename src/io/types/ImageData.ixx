module;

#include <vector>

export module helios.core.io:types.ImageData;

export namespace helios::core::io {
/**
 * @brief Simple container for loading byte-baseds image data along with meta-data information.
 */
struct ImageData {

    std::vector<std::byte> data;

    int width;

    int height;

    int nrChannels;
};

} // namespace helios::core::io
