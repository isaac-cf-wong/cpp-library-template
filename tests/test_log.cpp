/// @file
/// @brief Tests for the logger.

#include "cpp_library_template/log.hpp"

#include <array>
#include <cctype>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "cpp_library_template/version.hpp"

namespace {

using testing::HasSubstr;

TEST(LogLevelNames, RoundTripsThroughStrings) {
    for (const auto level : {cpp_library_template::LogLevel::trace,
                             cpp_library_template::LogLevel::debug,
                             cpp_library_template::LogLevel::info,
                             cpp_library_template::LogLevel::warning,
                             cpp_library_template::LogLevel::error,
                             cpp_library_template::LogLevel::critical,
                             cpp_library_template::LogLevel::off}) {
        EXPECT_EQ(cpp_library_template::level_from_string(cpp_library_template::to_string(level)), level);
    }
}

TEST(LogLevelNames, ParsingIsCaseInsensitive) {
    EXPECT_EQ(cpp_library_template::level_from_string("INFO"), cpp_library_template::LogLevel::info);
    EXPECT_EQ(cpp_library_template::level_from_string("Warning"), cpp_library_template::LogLevel::warning);
}

TEST(LogLevelNames, ParsingRejectsUnknownNames) {
    // The result is [[nodiscard]], so bind it rather than discarding it: the
    // build treats an ignored return value as an error.
    EXPECT_THROW(
        { [[maybe_unused]] const auto level = cpp_library_template::level_from_string("chatty"); },
        std::invalid_argument);
}

// A fixture, the analogue of a pytest fixture: each test gets a fresh stream and
// a logger writing into it.
class LoggerTest : public testing::Test {
protected:
    std::ostringstream sink_;
    cpp_library_template::Logger logger_{"test", sink_, cpp_library_template::LogLevel::info};

    [[nodiscard]] std::string output() const { return sink_.str(); }
};

/// @brief Assert that @p stamp is an ISO 8601 UTC timestamp, `2026-01-31T09:15:00Z`.
///
/// std::regex is avoided deliberately: gmock's MatchesRegex falls back to its own
/// simple regex flavour on some platforms, which supports `\d` but not `{n}`,
/// while POSIX supports `{n}` but not `\d`. Checking the layout is portable.
::testing::AssertionResult is_iso8601_utc(std::string_view stamp) {
    constexpr std::size_t kLength = 20;

    if (stamp.size() != kLength) {
        return ::testing::AssertionFailure()
               << "'" << stamp << "' is " << stamp.size() << " characters, expected " << kLength;
    }

    // Positions of the separators, and everything else must be a digit.
    static constexpr std::array<std::pair<std::size_t, char>, 6> kSeparators{
        {{4, '-'}, {7, '-'}, {10, 'T'}, {13, ':'}, {16, ':'}, {19, 'Z'}}};

    std::string expected_digits(kLength, 'd');
    for (const auto& [index, character] : kSeparators) {
        expected_digits[index] = character;
        if (stamp[index] != character) {
            return ::testing::AssertionFailure() << "'" << stamp << "' has '" << stamp[index] << "' at index " << index
                                                 << ", expected '" << character << "'";
        }
    }

    for (std::size_t index = 0; index < kLength; ++index) {
        if (expected_digits[index] != 'd') {
            continue;
        }
        if (std::isdigit(static_cast<unsigned char>(stamp[index])) == 0) {
            return ::testing::AssertionFailure() << "'" << stamp << "' has a non-digit at index " << index;
        }
    }

    return ::testing::AssertionSuccess();
}

TEST_F(LoggerTest, RecordCarriesTimestampLevelNameAndMessage) {
    logger_.info("hello");

    const std::string record = output();
    ASSERT_THAT(record, HasSubstr(" | info | test | hello\n"));

    EXPECT_TRUE(is_iso8601_utc(record.substr(0, record.find(" |"))));
}

TEST_F(LoggerTest, RecordsBelowTheActiveLevelAreDropped) {
    logger_.trace("dropped");
    logger_.debug("dropped");
    logger_.info("kept");

    EXPECT_THAT(output(), HasSubstr("kept"));
    EXPECT_THAT(output(), testing::Not(HasSubstr("dropped")));
}

TEST_F(LoggerTest, EveryLevelAboveTheThresholdIsEmitted) {
    logger_.warning("w");
    logger_.error("e");
    logger_.critical("c");

    EXPECT_THAT(output(), HasSubstr("| warning |"));
    EXPECT_THAT(output(), HasSubstr("| error |"));
    EXPECT_THAT(output(), HasSubstr("| critical |"));
}

TEST_F(LoggerTest, SetLevelChangesWhatIsEmitted) {
    logger_.set_level(cpp_library_template::LogLevel::trace);
    EXPECT_EQ(logger_.level(), cpp_library_template::LogLevel::trace);

    logger_.trace("now visible");
    EXPECT_THAT(output(), HasSubstr("now visible"));
}

TEST_F(LoggerTest, LevelOffSuppressesEverythingIncludingCritical) {
    logger_.set_level(cpp_library_template::LogLevel::off);

    logger_.critical("silenced");

    EXPECT_TRUE(output().empty());
}

// LogLevel::off is not a severity a caller can log *at*: passing it must be
// suppressed regardless of the active level, including the most verbose one.
TEST_F(LoggerTest, LoggingAtLevelOffIsNeverEmitted) {
    logger_.set_level(cpp_library_template::LogLevel::trace);

    logger_.log(cpp_library_template::LogLevel::off, "not a severity");

    EXPECT_TRUE(output().empty());
}

TEST_F(LoggerTest, ShouldLogAgreesWithWhatIsEmitted) {
    for (const auto level : {cpp_library_template::LogLevel::trace,
                             cpp_library_template::LogLevel::debug,
                             cpp_library_template::LogLevel::info,
                             cpp_library_template::LogLevel::warning,
                             cpp_library_template::LogLevel::error,
                             cpp_library_template::LogLevel::critical,
                             cpp_library_template::LogLevel::off}) {
        sink_.str({});
        const bool predicted = logger_.should_log(level);
        logger_.log(level, "probe");
        EXPECT_EQ(predicted, !sink_.str().empty()) << "level " << cpp_library_template::to_string(level);
    }
}

TEST_F(LoggerTest, NameIsReported) {
    EXPECT_EQ(logger_.name(), "test");
}

TEST_F(LoggerTest, VersionRecordNamesTheLibraryVersion) {
    logger_.log_version_information();

    EXPECT_THAT(output(), HasSubstr(std::string{cpp_library_template::version_full}));
}

class LoggerFileSinkTest : public LoggerTest {
protected:
    void SetUp() override {
        directory_ = std::filesystem::temp_directory_path() /
                     ("cpp_library_template_test_" + std::to_string(testing::UnitTest::GetInstance()->random_seed()) +
                      "_" + testing::UnitTest::GetInstance()->current_test_info()->name());
        std::filesystem::remove_all(directory_);
    }

    void TearDown() override {
        // Release the log file before deleting the directory. On POSIX an open
        // file can be unlinked; on Windows it cannot, and remove_all throws
        // "The process cannot access the file because it is being used by
        // another process". The logger outlives TearDown -- it is a member of
        // the fixture -- so it has to be told to let go.
        logger_.remove_file_sink();

        // Cleanup must not throw out of TearDown, where a failure would be
        // reported against whichever test ran, masking the real result.
        std::error_code error;
        std::filesystem::remove_all(directory_, error);
    }

    [[nodiscard]] static std::string read(const std::filesystem::path& path) {
        const std::ifstream file{path};
        std::ostringstream contents;
        contents << file.rdbuf();
        return contents.str();
    }

    std::filesystem::path directory_;
};

TEST_F(LoggerFileSinkTest, FileSinkMirrorsTheStreamAndCreatesParentDirectories) {
    const auto path = directory_ / "nested" / "run.log";
    logger_.add_file_sink(path);

    logger_.info("to both sinks");

    ASSERT_TRUE(std::filesystem::exists(path));
    EXPECT_EQ(read(path), output());
}

TEST_F(LoggerFileSinkTest, FileSinkAppendsRatherThanTruncating) {
    const auto path = directory_ / "run.log";
    std::filesystem::create_directories(directory_);
    {
        std::ofstream seed{path};
        seed << "earlier run\n";
    }

    logger_.add_file_sink(path);
    logger_.info("later run");

    const std::string contents = read(path);
    EXPECT_THAT(contents, HasSubstr("earlier run"));
    EXPECT_THAT(contents, HasSubstr("later run"));
}

TEST_F(LoggerFileSinkTest, DroppedRecordsAreNotWrittenToTheFile) {
    const auto path = directory_ / "run.log";
    logger_.add_file_sink(path);

    logger_.debug("below the threshold");

    EXPECT_TRUE(read(path).empty());
}

TEST_F(LoggerFileSinkTest, RemoveFileSinkStopsMirroringButKeepsTheStream) {
    const auto path = directory_ / "run.log";
    logger_.add_file_sink(path);
    logger_.info("to both sinks");

    logger_.remove_file_sink();
    logger_.info("stream only");

    const std::string contents = read(path);
    EXPECT_THAT(contents, HasSubstr("to both sinks"));
    EXPECT_THAT(contents, testing::Not(HasSubstr("stream only")));
    EXPECT_THAT(output(), HasSubstr("stream only"));
}

TEST_F(LoggerFileSinkTest, RemoveFileSinkIsSafeWithNoFileSinkAttached) {
    logger_.remove_file_sink();
    logger_.remove_file_sink();

    logger_.info("still works");
    EXPECT_THAT(output(), HasSubstr("still works"));
}

// The file must be deletable once the sink is released. This passes trivially on
// POSIX, where an open file can be unlinked anyway, and is the real check on
// Windows, where it cannot -- which is how this API came to exist.
TEST_F(LoggerFileSinkTest, TheLogFileCanBeDeletedAfterReleasingTheSink) {
    const auto path = directory_ / "run.log";
    logger_.add_file_sink(path);
    logger_.info("something");
    ASSERT_TRUE(std::filesystem::exists(path));

    logger_.remove_file_sink();

    std::error_code error;
    std::filesystem::remove(path, error);
    EXPECT_FALSE(error) << error.message();
    EXPECT_FALSE(std::filesystem::exists(path));
}

TEST_F(LoggerFileSinkTest, OpeningAnUnwritablePathThrows) {
    // A directory can never be opened as a log file.
    std::filesystem::create_directories(directory_ / "a_directory");

    EXPECT_THROW(logger_.add_file_sink(directory_ / "a_directory"), std::runtime_error);
}

TEST(LoggerDefaultSink, DefaultsToClogAndInfoLevel) {
    testing::internal::CaptureStderr();
    cpp_library_template::Logger logger{"default"};
    EXPECT_EQ(logger.level(), cpp_library_template::LogLevel::info);

    logger.debug("dropped");
    logger.info("kept");
    const std::string captured = testing::internal::GetCapturedStderr();

    EXPECT_THAT(captured, HasSubstr("| info | default | kept"));
    EXPECT_THAT(captured, testing::Not(HasSubstr("dropped")));
}

}  // namespace
