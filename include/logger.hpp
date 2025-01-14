// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/5/2024.

#pragma once

#include <chrono>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>

namespace c4lib {

class Logger {
public:
    Logger(const Logger&) = delete;

    Logger& operator=(const Logger&) = delete;

    Logger(Logger&&) noexcept = delete;

    Logger& operator=(Logger&&) noexcept = delete;

    enum class Severity { info, warn, error };

    static void error(const std::string& message) noexcept
    {
        log(Severity::error, message);
    }

    template<typename... Args> static void error(const std::format_string<Args...> fmt, Args&&... args) noexcept
    {
        instance().log(Severity::error, fmt, std::forward<Args>(args)...);
    }

    static void info(const std::string& message) noexcept
    {
        log(Severity::info, message);
    }

    template<typename... Args> static void info(const std::format_string<Args...> fmt, Args&&... args) noexcept
    {
        instance().log(Severity::info, fmt, std::forward<Args>(args)...);
    }

    static void set_threshold(Severity threshold) noexcept
    {
        instance().m_threshold = threshold;
    }

    static void start(const std::string& filename, Severity threshold);

    static void start(std::ostream& stream, Severity threshold);

    static void stop();

    static void warn(const std::string& message) noexcept
    {
        log(Severity::warn, message);
    }

    template<typename... Args> static void warn(const std::format_string<Args...> fmt, Args&&... args) noexcept
    {
        instance().log(Severity::warn, fmt, std::forward<Args>(args)...);
    }

private:
    static void log(Severity severity, const std::string& message) noexcept
    {
        if (severity >= instance().m_threshold) {
            try {
                const auto now{std::chrono::system_clock::now()};
                instance().m_out.get() << std::format("{:%m-%d-%Y %H:%M:%OS} UTC", now);
                instance().m_out.get() << " " << severity_to_string(severity) << ": ";
                instance().m_out.get() << message << "\n";
            }
            // NOLINTNEXTLINE(bugprone-empty-catch)
            catch (...) {
                // Logger is designed to be callable in destructors or within a try block.  For this reason,
                // we must not throw.  If an exception occurs, we'll have to silently ignore it.
            }
        }
    }

    template<typename... Args>
    static void log(Severity severity, const std::format_string<Args...> fmt, Args&&... args) noexcept
    {
        try {
            if (severity >= instance().m_threshold) {
                std::string message{std::vformat(fmt.get(), std::make_format_args(args...))};
                log(severity, message);
            }
        }
        // NOLINTNEXTLINE(bugprone-empty-catch)
        catch (...) {
            // Logger is designed to be callable in destructors or within a try block.  For this reason,
            // we must not throw.  If an exception occurs, we'll have to silently ignore it.
        }
    }

    static Logger& instance() noexcept
    {
        static Logger logger;
        return logger;
    }

    static std::string severity_to_string(Severity severity)
    {
        switch (severity) {
        case Severity::info:
            return "[INFO]";
        case Severity::warn:
            return "[WARNING]";
        case Severity::error:
        default:
            return "[ERROR]";
        }
    }

    Logger() = default;

    ~Logger() = default;

    std::ofstream m_file{};
    static std::fstream m_null_out;
    std::reference_wrapper<std::ostream> m_out{m_file};
    Severity m_threshold{Severity::info};
};

} // namespace c4lib
