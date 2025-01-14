// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/16/2024.

#pragma once

#include <lib/native/path.hpp>
#include <string>
#include <vector>

namespace c4lib {

class File_manager {
public:
    File_manager(native::Path install_root, native::Path custom_assets_path, std::string mod_name);

    ~File_manager() = default;

    File_manager(const File_manager&) = delete;   
	
    File_manager& operator=(const File_manager&) = delete;  
	
    File_manager(File_manager&&) noexcept = delete;   
	
    File_manager& operator=(File_manager&&) noexcept = delete; 

    void get_full_path(const native::Path& search_path, native::Path& full_path) const;

    void get_full_paths_modular(const native::Path& search_path, std::vector<native::Path>& full_paths) const;

private:
    native::Path m_custom_assets_path;
    native::Path m_install_root;
    std::string m_mod_name;
    native::Path m_modular_search_path_root;
    std::vector<native::Path> m_search_path_roots;
};

} // namespace c4lib
