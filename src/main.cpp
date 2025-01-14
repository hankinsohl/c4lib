// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 9/27/2024.

#include <array>
#include <boost/property_tree/ptree.hpp>
#include <cstddef>
#include <exception>
#include <include/c4lib.hpp>
#include <include/logger.hpp>
#include <iosfwd>
#include <iostream>
#include <lib/options/exceptions.hpp>
#include <lib/options/options-manager.hpp>
#include <lib/util/narrow.hpp>
#include <lib/util/options-data.hpp>
#include <lib/util/timer.hpp>
#include <span>
#include <src/options-data.hpp>
#include <src/options.hpp>
#include <src/text.hpp>
#include <src/util.hpp>
#include <sstream>
#include <string>

namespace bpt = boost::property_tree;
namespace edopt = c4edit::options;
namespace hopt = hankinsohl::options;
namespace libopt = c4lib::options;

namespace {
inline constexpr const char* log_filename{"c4lib-log.txt"};
}

int main(int argc, char* argv[])
{
    int rc{0};
    hopt::Options_manager options_manager;
    try {
        // Note: Output to cout is intentionally flushed because when running under the CLion IDE,
        // output that is not flushed does not appear until the program exits.
        // NOLINTNEXTLINE(performance-avoid-endl)
        std::cout << c4edit::banner() << std::endl;

        // Read options from command line/config file into options manager
        const std::span<const char* const> args{argv, gsl::narrow<size_t>(argc)};
        options_manager.add_options_from_command_line(args);
        auto options{options_manager.get_options()};
        if (options.contains(edopt::config_file)) {
            options_manager.add_options_from_config_file(options[edopt::config_file]);
        }

        // Add option info and aggregate check info from the exe and the library to the
        // options manager.
        options_manager.add_info(edopt::exe_options_info_lookup);
        options_manager.add_aggregate_checks({{edopt::requires_one_load_option, hopt::check_requires_at_least_one_of},
            {edopt::requires_one_write_option, hopt::check_requires_at_least_one_of},
            {edopt::multiple_load_options_are_incompatible, hopt::check_compatibility}});
        options_manager.add_info(libopt::lib_options_info_lookup);

        // Check the options
        options_manager.set_defaults_then_check_options();

        // Get options specific to the library
        auto lib_options{options_manager.get_options_exclusive_of(edopt::exe_options_info_lookup)};

        // Get all options
        options = options_manager.get_options();

        c4lib::Timer timer;
        timer.start();

        // Process log option
        if (options.contains(edopt::log) && options[edopt::log] == "1") {
            c4lib::Logger::start(log_filename, c4lib::Logger::Severity::info);
        }

        // Process open option
        std::string in_path;
        bpt::ptree ptree;
        if (options.contains(edopt::load_save)) {
            in_path = options[edopt::load_save];
            std::cout << c4edit::text::reading_save_from << ' ' << in_path << "... " << std::flush;
            c4lib::read_save(ptree, in_path, lib_options);
        }
        else if (options.contains(edopt::load_info)) {
            in_path = options[edopt::load_info];
            std::cout << c4edit::text::reading_info_from << ' ' << in_path << "... " << std::flush;
            c4lib::read_info(ptree, in_path, lib_options);
        }
        std::cout << c4edit::text::finished_in << ' ' << timer.to_string() << '\n' << std::flush;

        // Process write options
        const std::array write_options{c4edit::Write_option_info{.option = edopt::write_translation,
                                           .func = &c4lib::write_translation,
                                           .progress_message = c4edit::text::writing_translation_to},
            c4edit::Write_option_info{
                .option = edopt::write_info, .func = &c4lib::write_info, .progress_message = c4edit::text::writing_info_to},
            c4edit::Write_option_info{
                .option = edopt::write_save, .func = &c4lib::write_save, .progress_message = c4edit::text::writing_save_to}};
        for (const auto& write_option : write_options) {
            process_write_option(write_option, ptree, options, lib_options);
        }
    }
    catch (const hopt::Display_help_error& ex) {
        c4edit::display_help(ex.what());
    }
    catch (const hopt::Options_error& ex) {
        std::cerr << ex.what() << '\n';
        std::stringstream help;
        options_manager.write_help_message(help);
        c4edit::display_help(help.str());
        rc = -1;
    }
    catch (const std::exception& ex) {
        std::cerr << '\n' << ex.what() << '\n';
        rc = -1;
    }

    return rc;
}
