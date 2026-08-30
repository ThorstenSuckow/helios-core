module;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <string>
#include <vector>

export module helios.core.io:ImageReader;

import :FileReader;
import :types;

export namespace helios::core::io {
/**
 * @brief Simple ImageReader based in stb-image.
 *
 * @see https://github.com/nothings/stb
 */
class ImageReader : public FileReader<ImageData> {

public:
    [[nodiscard]] ImageData data(const std::string& filename) const override {

        int width;
        int height;
        int nrChannels;

        stbi_set_flip_vertically_on_load_thread(1);
        unsigned char* rawData = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);

        // we intentionally copy the file contents for clear ownership intent
        const auto size = width * height * nrChannels;
        std::vector<std::byte> pixels(size);
        std::memcpy(pixels.data(), rawData, size);
        stbi_image_free(rawData);

        return ImageData{.data = std::move(pixels), .width = width, .height = height, .nrChannels = nrChannels};
    };

    [[nodiscard]] bool readInto(const std::string& filename, ImageData& sink) const noexcept override {

        int width;
        int height;
        int nrChannels;

        try {
            stbi_set_flip_vertically_on_load_thread(1);
            unsigned char* rawData = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);

            // we intentionally copy the file contents for clear ownership intent
            const auto size = width * height * nrChannels;
            std::vector<std::byte> pixels(size);
            std::memcpy(pixels.data(), rawData, size);
            stbi_image_free(rawData);

            sink.data = std::move(pixels);
            sink.width = width;
            sink.height = height;
            sink.nrChannels = nrChannels;
            return true;

        } catch (std::exception& e) {
            logger_.error("Error while reading from target {0}: {1}", filename, e.what());
            return false;
        }
    };
};

} // namespace helios::core::io