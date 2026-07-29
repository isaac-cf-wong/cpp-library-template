#include "cpp_library_template/log.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "cpp_library_template/version.hpp"

namespace cpp_library_template {
namespace {

constexpr std::array<std::pair<LogLevel, std::string_view>, 7> kLevelNames{{
    {LogLevel::trace, "trace"},
    {LogLevel::debug, "debug"},
    {LogLevel::info, "info"},
    {LogLevel::warning, "warning"},
    {LogLevel::error, "error"},
    {LogLevel::critical, "critical"},
    {LogLevel::off, "off"},
}};

/// @brief Break @p time down into UTC calendar fields.
///
/// `std::gmtime` is not thread safe and MSVC deprecates it, so use the
/// reentrant platform variant. `std::chrono::zoned_time` formatting would be
/// nicer but is not available on every compiler this template supports.
[[nodiscard]] std::tm to_utc(std::time_t time) noexcept {
    std::tm parts{};
#ifdef _WIN32
    gmtime_s(&parts, &time);
#else
    gmtime_r(&time, &parts);
#endif
    return parts;
}

/// @brief ISO 8601 UTC timestamp with second resolution.
[[nodiscard]] std::string timestamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t seconds = std::chrono::system_clock::to_time_t(now);
    const std::tm parts = to_utc(seconds);

    std::ostringstream out;
    out << std::put_time(&parts, "%Y-%m-%dT%H:%M:%SZ");
    return out.str();
}

}  // namespace

std::string_view to_string(LogLevel level) noexcept {
    for (const auto& [value, name] : kLevelNames) {
        if (value == level) {
            return name;
        }
    }
    return "unknown";
}

LogLevel level_from_string(std::string_view name) {
    std::string lowered{name};
    std::ranges::transform(
        lowered, lowered.begin(), [](unsigned char character) { return static_cast<char>(std::tolower(character)); });

    for (const auto& [value, candidate] : kLevelNames) {
        if (candidate == lowered) {
            return value;
        }
    }
    throw std::invalid_argument{"unknown log level: " + std::string{name}};
}

Logger::Logger(std::string name, std::ostream& sink, LogLevel level)
    : name_{std::move(name)}, sink_{&sink}, level_{level} {}

Logger::Logger(std::string name, LogLevel level) : Logger{std::move(name), std::clog, level} {}

void Logger::add_file_sink(const std::filesystem::path& path) {
    if (path.has_parent_path()) {
        std::error_code error;
        std::filesystem::create_directories(path.parent_path(), error);
        if (error) {
            throw std::runtime_error{"cannot create log directory " + path.parent_path().string() + ": " +
                                     error.message()};
        }
    }

    auto stream = std::make_shared<std::ofstream>(path, std::ios::app);
    if (!stream->is_open()) {
        throw std::runtime_error{"cannot open log file " + path.string()};
    }
    file_sink_ = std::move(stream);
}

bool Logger::should_log(LogLevel level) const noexcept {
    if (level_ == LogLevel::off || level == LogLevel::off) {
        return false;
    }
    return static_cast<int>(level) >= static_cast<int>(level_);
}

void Logger::log(LogLevel level, std::string_view message) {
    if (!should_log(level)) {
        return;
    }

    std::ostringstream record;
    record << timestamp() << " | " << to_string(level) << " | " << name_ << " | " << message << '\n';
    const std::string text = record.str();

    *sink_ << text;
    sink_->flush();

    if (file_sink_) {
        *file_sink_ << text;
        file_sink_->flush();
    }
}

void Logger::log_version_information() {
    log(LogLevel::info, std::string{"cpp_library_template version "} + std::string{version_full});
}

}  // namespace cpp_library_template
