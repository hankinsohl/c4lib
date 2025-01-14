// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/16/2024.

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <lib/importer/file-manager.hpp>
#include <lib/native/path.hpp>
#include <lib/util/narrow.hpp>
#include <string>
#include <utility>
#include <vector>

namespace c4lib {

File_manager::File_manager(native::Path install_root, native::Path custom_assets_path, std::string mod_name)
    : m_custom_assets_path(std::move(custom_assets_path)),
      m_install_root(std::move(install_root)),
      m_mod_name(std::move(mod_name))
{
    const native::Path mn_as_path{m_mod_name};
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    m_search_path_roots.reserve(5);
    if (!m_mod_name.empty()) {
        m_modular_search_path_root = m_install_root / native::Path{"Mods"} / mn_as_path / native::Path{"Modules"};
        m_search_path_roots.emplace_back(
            m_install_root / native::Path{"Mods"} / mn_as_path / native::Path{"Assets/XML"});
    }
    m_search_path_roots.emplace_back(m_custom_assets_path / native::Path{"XML"});
    m_search_path_roots.emplace_back(m_install_root / native::Path{"Assets/XML"});
    m_search_path_roots.emplace_back(m_install_root / native::Path{"../Warlords/Assets/XML"});
    m_search_path_roots.emplace_back(m_install_root / native::Path{"../Assets/XML"});
}

void File_manager::get_full_path(const native::Path& search_path, native::Path& full_path) const
{
    for (const auto& root : m_search_path_roots) {
        full_path = root / search_path;
        std::filesystem::path const filePath{full_path};
        if (std::filesystem::is_regular_file(filePath)) {
            return;
        }
    }
    full_path.clear();
}

void File_manager::get_full_paths_modular(const native::Path& search_path, std::vector<native::Path>& full_paths) const
{
    full_paths.clear();

    // Extract the file name from the searchPath.
    std::string file_pattern{"_" + std::filesystem::path(search_path).filename().string()};

    // Windows considers two file names identical if they match character-for-character when compared
    // ignoring case.  We therefore convert file names to lowercase prior to comparison.
    std::ranges::transform(
        file_pattern, file_pattern.begin(), [](unsigned char c) { return gsl::narrow<char>(std::tolower(c)); });

    // Recursively search the modules directory for matching files.
    for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(m_modular_search_path_root)) {
        if (std::filesystem::is_regular_file(dir_entry)) {
            std::string filename{dir_entry.path().filename().string()};
            std::ranges::transform(
                filename, filename.begin(), [](unsigned char c) { return gsl::narrow<char>(std::tolower(c)); });
            if (filename.find(file_pattern) != std::string::npos) {
                full_paths.emplace_back(dir_entry.path());
            }
        }
    }
}

} // namespace c4lib
