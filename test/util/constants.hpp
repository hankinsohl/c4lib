// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 10/31/2024.

#pragma once

#include <lib/native/path.hpp>

namespace c4lib::test::constants {

inline const native::Path data_config_dir{"data/config"};
inline const native::Path data_misc_dir{"data/misc"};
inline const native::Path data_info_dir{"data/info"};
inline const native::Path data_saves_dir{"data/saves"};
inline const native::Path data_translations_dir{"data/translations"};
inline const native::Path relative_root_path{"../"};
inline const native::Path relative_root_path_doc{"../doc"};
inline const native::Path round_trip_test_dir{"out/round-trip-test"};
inline const native::Path out_dir{"out"};
inline const native::Path out_common_dir{"out/common"};
inline const native::Path out_c4edit_test_dir{"out/c4edit-test"};

inline const native::Path build_manifest{"latest-build.txt"};

} // namespace c4lib::test::constants
