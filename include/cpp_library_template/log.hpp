/// @file
/// @brief A minimal, dependency-free logger.
///
/// This mirrors the `utils/log.py` example of the Python template. It is a
/// deliberately small piece of real code -- enough to have branches worth
/// covering and an interface worth documenting -- not a logging framework. If
/// your library needs one, replace this file with spdlog and add the dependency
/// to `conanfile.py`.

#pragma once

#include <cstdint>
#include <filesystem>
#include <iosfwd>
#include <memory>
#include <string>
#include <string_view>

#include "cpp_library_template/export.hpp"

namespace cpp_library_template {

/// @brief Severity levels, ordered from most to least verbose.
///
/// The underlying type is fixed so the enum is one byte and its representation
/// does not depend on the compiler.
enum class LogLevel : std::uint8_t {
    trace,     ///< Fine-grained tracing, off by default.
    debug,     ///< Diagnostic detail for developers.
    info,      ///< Normal operational messages.
    warning,   ///< Something unexpected that was handled.
    error,     ///< An operation failed.
    critical,  ///< The process cannot continue.
    off,       ///< Suppress everything.
};

/// @brief Convert @p level to its lowercase name.
///
/// @param level Level to convert.
/// @return A static string such as `"info"`.
[[nodiscard]] CPP_LIBRARY_TEMPLATE_EXPORT std::string_view to_string(LogLevel level) noexcept;

/// @brief Parse a level name, case-insensitively.
///
/// @param name Level name, e.g. `"INFO"`, `"info"` or `"Warning"`.
/// @return The parsed level.
/// @throws std::invalid_argument if @p name is not a known level.
[[nodiscard]] CPP_LIBRARY_TEMPLATE_EXPORT LogLevel level_from_string(std::string_view name);

/// @brief A logger that writes timestamped records to a stream and, optionally,
///        to a file.
///
/// Records are formatted as `TIMESTAMP | LEVEL | name | message`. The logger is
/// not thread safe; guard it externally if you share one across threads.
class CPP_LIBRARY_TEMPLATE_EXPORT Logger {
public:
    /// @brief Construct a logger writing to @p sink.
    ///
    /// @param name  Logger name, included in every record.
    /// @param sink  Stream to write records to. The caller owns it and must
    ///              keep it alive for the lifetime of the logger.
    /// @param level Minimum level to emit.
    Logger(std::string name, std::ostream& sink, LogLevel level = LogLevel::info);

    /// @brief Construct a logger writing to `std::clog`.
    ///
    /// @param name  Logger name, included in every record.
    /// @param level Minimum level to emit.
    explicit Logger(std::string name, LogLevel level = LogLevel::info);

    /// @brief Also mirror every emitted record to @p path.
    ///
    /// Parent directories are created if needed. The file is opened in append
    /// mode, so restarting a process does not discard earlier records.
    ///
    /// @param path Path of the log file.
    /// @throws std::runtime_error if the file cannot be opened.
    void add_file_sink(const std::filesystem::path& path);

    /// @brief Minimum level currently emitted.
    /// @return The active level.
    [[nodiscard]] LogLevel level() const noexcept { return level_; }

    /// @brief Set the minimum level to emit.
    /// @param level New minimum level.
    void set_level(LogLevel level) noexcept { level_ = level; }

    /// @brief Logger name included in every record.
    /// @return The name given at construction.
    [[nodiscard]] const std::string& name() const noexcept { return name_; }

    /// @brief Whether a record at @p level would be emitted.
    ///
    /// Use this to skip building an expensive message.
    ///
    /// @param level Level to test.
    /// @return `true` if the record would be emitted.
    [[nodiscard]] bool should_log(LogLevel level) const noexcept;

    /// @brief Emit @p message at @p level.
    ///
    /// @param level   Severity of the record.
    /// @param message Message body.
    void log(LogLevel level, std::string_view message);

    /// @brief Emit @p message at LogLevel::trace.
    /// @param message Message body.
    void trace(std::string_view message) { log(LogLevel::trace, message); }

    /// @brief Emit @p message at LogLevel::debug.
    /// @param message Message body.
    void debug(std::string_view message) { log(LogLevel::debug, message); }

    /// @brief Emit @p message at LogLevel::info.
    /// @param message Message body.
    void info(std::string_view message) { log(LogLevel::info, message); }

    /// @brief Emit @p message at LogLevel::warning.
    /// @param message Message body.
    void warning(std::string_view message) { log(LogLevel::warning, message); }

    /// @brief Emit @p message at LogLevel::error.
    /// @param message Message body.
    void error(std::string_view message) { log(LogLevel::error, message); }

    /// @brief Emit @p message at LogLevel::critical.
    /// @param message Message body.
    void critical(std::string_view message) { log(LogLevel::critical, message); }

    /// @brief Emit a single record naming the library and its version.
    ///
    /// Handy as the first line of a program's output when triaging bug reports.
    void log_version_information();

private:
    std::string name_;
    std::ostream* sink_;
    LogLevel level_;
    std::shared_ptr<std::ostream> file_sink_;
};

}  // namespace cpp_library_template
