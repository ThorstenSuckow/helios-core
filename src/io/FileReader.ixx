/**
 * @file FileReader.ixx
 * @brief Abstract base class for reading file contents.
 */
module;

#include <string>

export module helios.core.io:FileReader;

import helios.core.log.Logger;
import helios.core.log.LogManager;

#define HELIOS_LOG_SCOPE "helios::core::io.FileReader"
export namespace helios::core::io {


    /**
     * @brief Abstract base class for reading file contents.
     *
     */
    template<typename TData>
    class FileReader {

    protected:
        /**
         * @brief Shared logger instance for all FileReader objects.
         */
        inline static const helios::core::log::Logger& logger_ = helios::core::log::LogManager::loggerForScope(HELIOS_LOG_SCOPE);


    public:
        virtual ~FileReader() = default;

        /**
         * Default constructor.
         */
        FileReader() = default;

        /**
         * @brief Reads the entire contents of the file and returns them as a TData object.
         *
         * @param filename The path to the file to read.
         * @return The contents of the file as a TData object. If reading fails, a default-constructed TData is returned.
         *
         * @throws std::runtime_error if the file operation fails.
         */
        [[nodiscard]] virtual TData data(const std::string& filename) const  = 0;

        /**
         * @brief Attempts to read the entire contents of the file into the provided TData object.
         *
         * @param filename The path to the file to read.
         * @param sink Output parameter receiving the file contents on success.
         * @return true if reading succeeded and `contents` was populated, otherwise false.
         */
        [[nodiscard]] virtual bool readInto(const std::string& filename,  TData& sink) const noexcept = 0;


    };


}