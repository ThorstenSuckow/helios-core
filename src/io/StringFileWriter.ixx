/**
 * @file StringFileWriter.ixx
 * @brief Abstract base class for writing lines of text into a file.
 */
module;

#include <ostream>
#include <string>

export module helios.core.io.StringFileWriter;

import helios.core.log.Logger;
import helios.core.log.LogManager;

#define HELIOS_LOG_SCOPE "helios::core::io.FileWriter"
export namespace helios::core::io {

/**
 * @brief Abstract base class for writing a line of text into a file.
 */
class StringFileWriter {

protected:
    /**
     * @brief Shared logger instance for all StringFileWriter objects.
     */
    inline static const helios::core::log::Logger& logger_ =
        helios::core::log::LogManager::loggerForScope(HELIOS_LOG_SCOPE);

public:
    virtual ~StringFileWriter() = default;

    /**
     * Default constructor.
     */
    StringFileWriter() = default;

    /**
     * @brief Writes a single line to the file. A trailing newline is appended.
     *
     * @param filename The path to the file to write.
     * @param line The line to write (without line terminator).
     * @param append If true, the line is appended to the file; otherwise the file is truncated first.
     *
     * @throws std::runtime_error if the file cannot be opened or written.
     */
    virtual void writeLine(const std::string& filename, const std::string& line, bool append = true) const = 0;

    /**
     * @brief Attempts to write a single line to the file.
     *
     * @param filename The path to the file to write.
     * @param line The line to write (without line terminator).
     * @param append If true, the line is appended to the file; otherwise the file is truncated first.
     *
     * @return true if the line was written successfully, otherwise false.
     */
    [[nodiscard]] virtual bool tryWriteLine(
        const std::string& filename, const std::string& line, bool append = true
    ) const noexcept = 0;

    /**
     * @brief Writes a single line to an already opened stream. A trailing newline is appended.
     *
     * Avoids re-opening the file for every line. The caller owns the stream and is responsible
     * for opening, flushing and closing it.
     *
     * @param stream The opened output stream.
     * @param line The line to write (without line terminator).
     *
     * @throws std::runtime_error if the stream is not in a good state or writing fails.
     */
    virtual void writeLine(std::ostream& stream, const std::string& line) const = 0;

    /**
     * @brief Attempts to write a single line to an already opened stream.
     *
     * @param stream The opened output stream.
     * @param line The line to write (without line terminator).
     *
     * @return true if the line was written successfully, otherwise false.
     */
    [[nodiscard]] virtual bool tryWriteLine(std::ostream& stream, const std::string& line) const noexcept = 0;
};

} // namespace helios::core::io



