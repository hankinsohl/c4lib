// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/29/2024.

#include <format>
#include <include/exceptions.hpp>
#include <lib/util/exception-formats.hpp>
#include <lib/util/schema.hpp>
#include <string>

namespace c4lib {

std::string enum_name_from_type(const std::string& type_name)
{
    // The type_name string will be of the form enumXXX_<enum_name>Optional[...].  To extract the enum name we
    // take the substring starting one character after _ up until [ (or the end of the string).
    const std::string::size_type pos_underscore{type_name.find_first_of('_')};
    if (pos_underscore == std::string::npos || pos_underscore + 1 == type_name.length()) {
        throw Parser_error(std::format(fmt::bad_type_enum_format, type_name));
    }
    const std::string::size_type pos_left_bracket{type_name.find_first_of('[', pos_underscore)};
    return type_name.substr(pos_underscore + 1, pos_left_bracket - pos_underscore - 1);
}

std::string identifier_from_type(const std::string& type_name)
{
    // Returns the identifier portion of a type_name.  Used to extract the name of an enum,
    // struct or template from type_name.  Throws an exception if called on other types.
    // For enums, structs and templates the format of type_name is
    //    <(enum_ | struct_ | template_)><identifier><optional>
    // where optional is either empty or begins with [ for arrays or < for templates.
    const std::string::size_type pos_underscore{type_name.find_first_of('_')};
    if (pos_underscore == std::string::npos) {
        throw Parser_error(std::format(fmt::bad_type_underscore_missing, type_name));
    }
    const std::string::size_type pos_end_of_identifier{type_name.find_first_of("[<", pos_underscore)};
    return type_name.substr(pos_underscore + 1, pos_end_of_identifier - pos_underscore - 1);
}

std::string size_from_type(const std::string& type_name)
{
    // The type_name string will be of the form <type_name>XXX<(_ | [ | NULL)>... where XXX is the size in
    // bits. To extract the size we take the substring starting at the first digit and ending at (_ | [ | NULL).
    // Finally, we divide the extracted size by 8 to covert bit-size to byte-size.
    const std::string::size_type pos_first_digit{type_name.find_first_of("0123456789")};
    if (pos_first_digit == std::string::npos) {
        throw Parser_error(std::format(fmt::bad_type_size_missing, type_name));
    }
    const std::string::size_type pos_end{type_name.find_first_of("_[", pos_first_digit)};
    const std::string bit_size{type_name.substr(pos_first_digit, pos_end - pos_first_digit)};
    if (bit_size != "8" && bit_size != "16" && bit_size != "32") {
        throw Parser_error(std::format(fmt::bad_type_invalid_size, type_name));
    }
    constexpr int bits_per_byte{8};
    std::string byte_size{std::to_string(std::stoi(bit_size) / bits_per_byte)};
    return byte_size;
}

} // namespace c4lib
