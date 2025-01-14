// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/5/2024.

#include <format>
#include <fstream>
#include <include/logger.hpp>
#include <lib/native/path.hpp>
#include <lib/util/exception-formats.hpp>
#include <ostream>
#include <string>

namespace c4lib {
std::fstream Logger::m_null_out{};

void Logger::start(const std::string& filename, Severity threshold)
{
    const native::Path path{filename};
    instance().m_file.close();
    instance().m_file.clear();
    instance().m_file.open(path.c_str(), std::ofstream::out | std::ofstream::app);
    if (!instance().m_file) {
        throw std::runtime_error{std::format(fmt::runtime_error_opening_file, filename)};
    }

    instance().m_out.get().flush();
    instance().m_out = instance().m_file;
    instance().m_threshold = threshold;
}

void Logger::start(std::ostream& stream, Severity threshold)
{
    // In case our current stream is a file we've opened
    instance().m_file.close();
    instance().m_file.clear();

    instance().m_out.get().flush();
    instance().m_out = stream;
    instance().m_threshold = threshold;
}

void Logger::stop()
{
    instance().m_file.close();
    instance().m_file.clear();

    instance().m_out.get().flush();
    instance().m_out = m_null_out;
}

} // namespace c4lib
