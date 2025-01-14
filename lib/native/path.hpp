// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 12/23/2024.

#pragma once

#include <algorithm>
#include <format>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>
#include <lib/util/narrow.hpp>

namespace c4lib::native {

#ifdef linux
inline constexpr char directory_separator{'/'};
inline constexpr const char* directory_separator_string{"/"};
#else
inline constexpr char directory_separator{'\\'};
inline constexpr const char* directory_separator_string{"\\"};
#endif

class Path {
    friend std::formatter<Path>;

public:
    Path() = default;

    explicit Path(const char* path) : m_path(path)
    {
        make_path_native_(m_path);
    }

    explicit Path(std::string  path) : m_path(std::move(path))
    {
        make_path_native_(m_path);
    }

    explicit Path(const std::filesystem::path& path)
    {
        m_path = path.string();
        make_path_native_(m_path);
    }

    Path operator/(const Path& rhs) const
    {
        return Path{m_path + make_separator_(rhs) + rhs.m_path};
    }

    Path& operator/=(const Path& rhs)
    {
        m_path = m_path + make_separator_(rhs) + rhs.m_path;
        return *this;
    }

    // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
    operator std::string() const
    {
        return m_path;
    }

    // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
    operator std::filesystem::path() const
    {
        return std::filesystem::path{m_path};
    }

    Path& append(const std::string& ext)
    {
        m_path += ext;
        return *this;
    }

    [[nodiscard]] Path append_to_copy(const std::string& ext) const
    {
        Path copy(m_path);
        copy.m_path += ext;
        return copy;
    }

    void clear()
    {
        m_path.clear();
    }

    [[nodiscard]] bool empty() const
    {
        return m_path.empty();
    }

    [[nodiscard]] const std::string& str() const
    {
        return m_path;
    }

    [[nodiscard]] const char* c_str() const
    {
        return m_path.c_str();
    }

private:
    static void make_path_native_(std::string& path)
    {
        if (path.empty()) {
            return;
        }

        // Change path root from Linux "/mnt/<lowercase-drive-letter>/" to Windows "<drive-letter>:\"
        if (constexpr std::string_view mnt{"/mnt/"}; path.starts_with(mnt) && path.length() >= mnt.length()) {
            path = path.substr(mnt.length(), 1) + ":" + path.substr(mnt.length() + 1);
        }

        std::ranges::replace(path, '/', '\\');

#ifdef linux
        // Change path root from Windows "<drive-letter>:\" to Linux "/mnt/<lowercase-drive-letter>/"
        if (path.find(":\\") == 1) {
            path[0] = gsl::narrow<char>(std::tolower(gsl::narrow<unsigned char>(path[0])));
            path.erase(1, 1);
            path = "/mnt/" + path;
        }

        std::ranges::replace(path, '\\', '/');
#endif
        if (path.back() == directory_separator) {
            path.pop_back();
        }
    }

    [[nodiscard]] std::string make_separator_(const native::Path& rhs) const
    {
        std::string separator;
        if (!empty() && !rhs.empty() && rhs.m_path[0] != directory_separator) {
            separator = directory_separator_string;
        }
        return separator;
    }

    std::string m_path;
};

} // namespace c4lib::native

template<> struct std::formatter<c4lib::native::Path, char> : std::formatter<std::string> {
    static auto format(const c4lib::native::Path& p, std::format_context& ctx)
    {
        return std::format_to(ctx.out(), "{}", p.m_path);
    }
};
