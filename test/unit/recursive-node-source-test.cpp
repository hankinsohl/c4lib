// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 12/1/2024.

#include <boost/property_tree/ptree.hpp>
#include <gtest/gtest.h>
#include <include/c4lib.hpp>
#include <include/exceptions.hpp>
#include <include/node-attributes.hpp>
#include <lib/ptree/recursive-node-source.hpp>
#include <lib/util/exception-formats.hpp>
#include <string>
#include <test/util/macros.hpp>
#include <unordered_map>

namespace bpt = boost::property_tree;

namespace {

template<typename T> void unit_test_simple_recursion_impl(T it)
{
    EXPECT_EQ((*it).first, 0);
    EXPECT_STREQ((*it).second.template get_value<std::string>().c_str(), "1.1");
    ++it;
    EXPECT_EQ((*it).first, 1);
    EXPECT_STREQ((*it).second.template get_value<std::string>().c_str(), "2.1");
    ++it;
    EXPECT_EQ((*it).first, 1);
    EXPECT_STREQ((*it).second.template get_value<std::string>().c_str(), "2.2");
    ++it;
    EXPECT_EQ((*it).first, 1);
    EXPECT_STREQ((*it).second.template get_value<std::string>().c_str(), "2.3");
    ++it;
    EXPECT_EQ((*it).first, 0);
    EXPECT_STREQ((*it).second.template get_value<std::string>().c_str(), "1.2");
    ++it;
    EXPECT_EQ((*it).first, 0);
    EXPECT_STREQ((*it).second.template get_value<std::string>().c_str(), "1.3");
}

template<typename T> void unit_test_recursion_skip_attributes_child_impl(T it)
{
    EXPECT_EQ((*it).first, 0);
    EXPECT_STREQ((*it).second.template get_value<std::string>().c_str(), "1.1");
    ++it;
    EXPECT_EQ((*it).first, 1);
    EXPECT_STREQ((*it).second.template get_value<std::string>().c_str(), "2.1");
    ++it;
    EXPECT_EQ((*it).first, 1);
    EXPECT_STREQ((*it).second.template get_value<std::string>().c_str(), "2.2");
    ++it;
    EXPECT_EQ((*it).first, 1);
    EXPECT_STREQ((*it).second.template get_value<std::string>().c_str(), "2.3");
    ++it;
    EXPECT_EQ((*it).first, 0);
    EXPECT_STREQ((*it).second.template get_value<std::string>().c_str(), "1.3");
}

template<typename T> void unit_test_recursion_skip_attributes_parent_impl(T it)
{
    EXPECT_EQ((*it).first, 0);
    EXPECT_STREQ((*it).second.template get_value<std::string>().c_str(), "1.2");
    ++it;
    EXPECT_EQ((*it).first, 0);
    EXPECT_STREQ((*it).second.template get_value<std::string>().c_str(), "1.3");
}

template<typename T> void unit_test_recursion_skip_multiple_attributes_parent_impl(T it)
{
    EXPECT_EQ((*it).first, 0);
    EXPECT_STREQ((*it).second.template get_value<std::string>().c_str(), "1.3");
    ++it;
    EXPECT_EQ((*it).first, 1);
    EXPECT_STREQ((*it).second.template get_value<std::string>().c_str(), "2.2");
    ++it;
    EXPECT_EQ((*it).first, 1);
    EXPECT_STREQ((*it).second.template get_value<std::string>().c_str(), "2.3");
    ++it;
    EXPECT_EQ((*it).first, 0);
    EXPECT_STREQ((*it).second.template get_value<std::string>().c_str(), "1.4");
    ++it;
    EXPECT_EQ((*it).first, 1);
    EXPECT_STREQ((*it).second.template get_value<std::string>().c_str(), "2.4");
    ++it;
    EXPECT_THROW_CONTAINS_MSG(*it, c4lib::Iterator_error, c4lib::fmt::dereference_of_iterator_at_end);
}

template<typename T> void unit_test_recursion_dereference_end_impl(T it)
{
    EXPECT_EQ((*it).first, 0);
    EXPECT_STREQ((*it).second.template get_value<std::string>().c_str(), "1.1");
    ++it;
    EXPECT_EQ((*it).first, 0);
    EXPECT_STREQ((*it).second.template get_value<std::string>().c_str(), "1.2");
    ++it;
    EXPECT_EQ((*it).first, 0);
    EXPECT_STREQ((*it).second.template get_value<std::string>().c_str(), "1.3");
    ++it;
    EXPECT_THROW_CONTAINS_MSG(*it, c4lib::Iterator_error, c4lib::fmt::dereference_of_iterator_at_end);
}

template<typename T> void unit_test_ranged_for_impl(T node_source)
{
    for (const auto& [depth, node] : node_source) {
        EXPECT_NO_THROW(static_cast<void>(node.empty()));
    }
}

template<typename T> void unit_test_recursion_using_ranged_for_impl(T node_source)
{
    int i{0};
    for (const auto& pr : node_source) {
        if (i == 0) {
            // Savegame node
            EXPECT_EQ(pr.first, 0);
            EXPECT_STREQ(pr.second.template get_value<std::string>().c_str(), "");
        }
        else if (i == 1) {
            // GameHeader node
            EXPECT_EQ(pr.first, 1);
            EXPECT_STREQ(pr.second.template get_value<std::string>().c_str(), "");
        }
        else {
            break;
        }
        ++i;
    }
    EXPECT_EQ(i, 2);
}

} // namespace

namespace c4lib::property_tree {

class Recursive_node_source_test : public testing::Test {
public:
    Recursive_node_source_test() = default;

    ~Recursive_node_source_test() override = default;

    Recursive_node_source_test(const Recursive_node_source_test&) = delete;

    Recursive_node_source_test& operator=(const Recursive_node_source_test&) = delete;

    Recursive_node_source_test(Recursive_node_source_test&&) noexcept = delete;

    Recursive_node_source_test& operator=(Recursive_node_source_test&&) noexcept = delete;

protected:
    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(Recursive_node_source_test, unit_test_simple_recursion)
{
    // Create a tree to test
    bpt::ptree root{"root"};
    root.add_child("Level_1_1", bpt::ptree{"1.1"});
    root.add_child("Level_1_2", bpt::ptree{"1.2"});
    root.add_child("Level_1_3", bpt::ptree{"1.3"});

    root.add_child("Level_1_1.Level_2_1", bpt::ptree{"2.1"});
    root.add_child("Level_1_1.Level_2_2", bpt::ptree{"2.2"});
    root.add_child("Level_1_1.Level_2_3", bpt::ptree{"2.3"});

    const Recursive_node_source node_source{&root};
    auto it{node_source.begin()};
    unit_test_simple_recursion_impl(it);
    auto it_c{node_source.cbegin()};
    unit_test_simple_recursion_impl(it_c);

    const bpt::ptree& const_root{root};
    const Recursive_node_source const_node_source{&const_root, skip_meta_nodes};
    auto it_c2{const_node_source.cbegin()};
    unit_test_simple_recursion_impl(it_c2);
}

TEST_F(Recursive_node_source_test, unit_test_recursion_skip_attributes_child)
{
    // Create a tree to test
    bpt::ptree root{"root"};
    root.add_child("Level_1_1", bpt::ptree{"1.1"});
    root.add_child("Level_1_2", bpt::ptree{nv_meta});
    root.add_child("Level_1_3", bpt::ptree{"1.3"});

    root.add_child("Level_1_1.Level_2_1", bpt::ptree{"2.1"});
    root.add_child("Level_1_1.Level_2_2", bpt::ptree{"2.2"});
    root.add_child("Level_1_1.Level_2_3", bpt::ptree{"2.3"});

    const Recursive_node_source node_source{&root, skip_meta_nodes};
    auto it{node_source.begin()};
    unit_test_recursion_skip_attributes_child_impl(it);
    auto it_c{node_source.cbegin()};
    unit_test_recursion_skip_attributes_child_impl(it_c);

    const bpt::ptree& const_root{root};
    const Recursive_node_source const_node_source{&const_root, skip_meta_nodes};
    auto it_c2{const_node_source.cbegin()};
    unit_test_recursion_skip_attributes_child_impl(it_c2);
}

TEST_F(Recursive_node_source_test, unit_test_recursion_skip_attributes_parent)
{
    // Create a tree to test
    bpt::ptree root{"root"};
    root.add_child("Level_1_1", bpt::ptree{nv_meta});
    root.add_child("Level_1_2", bpt::ptree{"1.2"});
    root.add_child("Level_1_3", bpt::ptree{"1.3"});

    root.add_child("Level_1_1.Level_2_1", bpt::ptree{"2.1"});
    root.add_child("Level_1_1.Level_2_2", bpt::ptree{"2.2"});
    root.add_child("Level_1_1.Level_2_3", bpt::ptree{"2.3"});

    const Recursive_node_source node_source{&root, skip_meta_nodes};
    auto it{node_source.begin()};
    unit_test_recursion_skip_attributes_parent_impl(it);
    auto it_c{node_source.cbegin()};
    unit_test_recursion_skip_attributes_parent_impl(it_c);

    const bpt::ptree& const_root{root};
    const Recursive_node_source const_node_source{&const_root, skip_meta_nodes};
    auto it_c2{const_node_source.cbegin()};
    unit_test_recursion_skip_attributes_parent_impl(it_c2);
}

TEST_F(Recursive_node_source_test, unit_test_recursion_skip_multiple_attributes_parent)
{
    // Create a tree to test
    bpt::ptree root{"root"};
    root.add_child("Level_1_1", bpt::ptree{nv_meta});
    root.add_child("Level_1_2", bpt::ptree{nv_meta});
    root.add_child("Level_1_3", bpt::ptree{"1.3"});
    root.add_child("Level_1_4", bpt::ptree{"1.4"});

    root.add_child("Level_1_3.Level_2_1", bpt::ptree{nv_meta});
    root.add_child("Level_1_3.Level_2_2", bpt::ptree{"2.2"});
    root.add_child("Level_1_3.Level_2_3", bpt::ptree{"2.3"});

    root.add_child("Level_1_4.Level_2_4", bpt::ptree{"2.4"});

    const Recursive_node_source node_source{&root, skip_meta_nodes};
    auto it{node_source.begin()};
    unit_test_recursion_skip_multiple_attributes_parent_impl(it);
    auto it_c{node_source.cbegin()};
    unit_test_recursion_skip_multiple_attributes_parent_impl(it_c);

    const bpt::ptree& const_root{root};
    const Recursive_node_source const_node_source{&const_root, skip_meta_nodes};
    auto it_c2{const_node_source.cbegin()};
    unit_test_recursion_skip_multiple_attributes_parent_impl(it_c2);
}

TEST_F(Recursive_node_source_test, unit_test_recursion_dereference_end)
{
    // Create a tree to test
    bpt::ptree root("root");
    root.add_child("Level_1_1", bpt::ptree{"1.1"});
    root.add_child("Level_1_2", bpt::ptree{"1.2"});
    root.add_child("Level_1_3", bpt::ptree{"1.3"});

    const Recursive_node_source node_source{&root, skip_meta_nodes};
    auto it{node_source.begin()};
    unit_test_recursion_dereference_end_impl(it);
    auto it_c{node_source.cbegin()};
    unit_test_recursion_dereference_end_impl(it_c);

    const bpt::ptree& const_root{root};
    const Recursive_node_source const_node_source{&const_root, skip_meta_nodes};
    auto it_c2{const_node_source.cbegin()};
    unit_test_recursion_dereference_end_impl(it_c2);
}

TEST_F(Recursive_node_source_test, unit_test_ranged_for)
{
    // Create a tree to test
    bpt::ptree root{"root"};
    root.add_child("Level_1_1", bpt::ptree{"1.1"});
    root.add_child("Level_1_2", bpt::ptree{"1.2"});
    root.add_child("Level_1_3", bpt::ptree{"1.3"});

    const Recursive_node_source node_source{&root, skip_meta_nodes};
    unit_test_ranged_for_impl(node_source);
    const bpt::ptree& const_root{root};
    const Recursive_node_source const_node_source{&const_root, skip_meta_nodes};
    unit_test_ranged_for_impl(const_node_source);
}

TEST_F(Recursive_node_source_test, unit_test_recursion_using_ranged_for)
{
    // Create a tree to test
    bpt::ptree root("root");
    root.add_child("__Origin__", bpt::ptree{nv_meta});
    root.add_child("Savegame", bpt::ptree{});
    root.add_child("Savegame.__Attributes__", bpt::ptree{nv_meta});
    root.add_child("Savegame.GameHeader", bpt::ptree{});

    // Test ranged for
    const Recursive_node_source node_source{&root, skip_meta_nodes};
    unit_test_recursion_using_ranged_for_impl(node_source);
    const bpt::ptree& const_root{root};
    const Recursive_node_source const_node_source{&const_root, skip_meta_nodes};
    unit_test_recursion_using_ranged_for_impl(const_node_source);
}

TEST_F(Recursive_node_source_test, DISABLED_unit_test_recursion_using_civ4_ptree)
{
    // Load the info for a civ4 save
    bpt::ptree ptree;
    static constexpr const char* brennus_2_info{R"(data/Brennus BC-4000-2.info)"};
    std::unordered_map<std::string, std::string> options{};
    EXPECT_NO_THROW(read_info(ptree, brennus_2_info, options));

    const Recursive_node_source node_source{&ptree, skip_meta_nodes};
    auto it = node_source.begin();

    // Savegame node
    EXPECT_EQ((*it).first, 0);
    EXPECT_STREQ((*it).second.get_value<std::string>().c_str(), "");

    // GameHeader node
    ++it;
    EXPECT_EQ((*it).first, 1);
    EXPECT_STREQ((*it).second.get_value<std::string>().c_str(), "");

    // GameVersion node
    ++it;
    EXPECT_EQ((*it).first, 2);
    EXPECT_STREQ((*it).second.get_value<std::string>().c_str(), "");

    const Recursive_node_source node_source2(&ptree, skip_meta_nodes);
    int i{0};
    for (const auto& pr : node_source2) {
        if (i == 0) {
            // Savegame node
            EXPECT_EQ(pr.first, 0);
            EXPECT_STREQ(pr.second.get_value<std::string>().c_str(), "");
        }
        else if (i == 1) {
            // GameHeader node
            EXPECT_EQ(pr.first, 1);
            EXPECT_STREQ(pr.second.get_value<std::string>().c_str(), "");
        }
        else if (i == 2) {
            // GameVersion node
            EXPECT_EQ(pr.first, 2);
            EXPECT_STREQ(pr.second.get_value<std::string>().c_str(), "");
        }
        else {
            break;
        }
        ++i;
    }
    EXPECT_EQ(i, 3);
}

} // namespace c4lib::property_tree
