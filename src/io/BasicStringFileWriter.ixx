/**
 * @file BasicStringFileWriter.ixx
 * @brief Basic line-to-file writer implementation.
 */
module;

#include <format>
#include <fstream>
#include <ios>
#include <ostream>
#include <stdexcept>
#include <string>

export module helios.core.io.BasicStringFileWriter;

import helios.core.io.StringFileWriter;

export namespace helios::core::io {

/**
 * @brief Basic implementation of a string file writer.
 */
class BasicStringFileWriter : public StringFileWriter {

public:
    /**
     * @copydoc StringFileWriter::writeLine(const std::string&, const std::string&, bool)
     */
    void writeLine(const std::string& filename, const std::string& line, const bool append = true) const override {
        std::ofstream outfile{filename, append ? std::ios::out | std::ios::app : std::ios::out | std::ios::trunc};

        if (!outfile) {
            const std::string msg = std::format("Cannot open {0}", filename);
            logger_.error(msg);
            throw std::runtime_error(msg);
        }

        writeLine(outfile, line);
    }

    /**
     * @copydoc StringFileWriter::tryWriteLine(const std::string&, const std::string&, bool)
     */
    [[nodiscard]] bool tryWriteLine(
        const std::string& filename, const std::string& line, const bool append = true
    ) const noexcept override {
        std::ofstream outfile{filename, append ? std::ios::out | std::ios::app : std::ios::out | std::ios::trunc};

        if (!outfile) {
            logger_.error(std::format("Cannot open {0}", filename));
            return false;
        }

        return tryWriteLine(outfile, line);
    }

    /**
     * @copydoc StringFileWriter::writeLine(std::ostream&, const std::string&)
     */
    void writeLine(std::ostream& stream, const std::string& line) const override {
        stream << line << '\n';

        if (!stream) {
            const std::string msg = "Cannot write to stream";
            logger_.error(msg);
            throw std::runtime_error(msg);
        }
    }

    /**
     * @copydoc StringFileWriter::tryWriteLine(std::ostream&, const std::string&)
     */
    [[nodiscard]] bool tryWriteLine(std::ostream& stream, const std::string& line) const noexcept override {
        stream << line << '\n';

        if (!stream) {
            logger_.error("Cannot write to stream");
            return false;
        }

        return true;
    }
};

} // namespace helios::core::io




