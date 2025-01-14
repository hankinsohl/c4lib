// Copyright (c) 2025 By David "Hankinsohl" Hankins.
// This software is licensed under the MIT License.
// Created by Hankinsohl on 11/30/2024.

#pragma once

#include <boost/property_tree/ptree.hpp>
#include <include/exceptions.hpp>
#include <include/node-attributes.hpp>
#include <lib/util/exception-formats.hpp>
#include <stack>
#include <type_traits>
#include <utility>
#include <boost/property_tree/ptree_fwd.hpp>

namespace c4lib::property_tree {

inline bool skip_meta_nodes(const boost::property_tree::ptree& node)
{
    return node.data() == nv_meta;
}

inline bool skip_none(const boost::property_tree::ptree&)
{
    return false;
}

template<typename P> class Recursive_node_source {
public:
    explicit Recursive_node_source(P* ptree, bool (*filter)(const boost::property_tree::ptree& ptree) = skip_none)
        : m_filter(filter), m_ptree(ptree)
    {}

    template<typename T> class base_iterator {
    public:
        using boost_iterator = std::conditional_t<std::is_const_v<T>,
            boost::property_tree::ptree::const_iterator,
            boost::property_tree::ptree::iterator>;

        base_iterator()
            : base_iterator(nullptr, nullptr)
        {}

        base_iterator(T* ptree, bool (*filter)(const boost::property_tree::ptree& ptree))
            : m_filter(filter == nullptr ? skip_none : filter)
        {
            if (ptree != nullptr) {
                boost_iterator begin{ptree->begin()};
                boost_iterator it{next_(ptree, begin)};
                if (it != ptree->end()) {
                    m_stack.emplace(ptree, it, true);
                }
            }
        }

        base_iterator& operator++()
        {
            while (!m_stack.empty()) {
                // Obtain the current context from the stack.
                Context& context = m_stack.top();

                // If we haven't yet processed this node's children, do so.
                if (context.fresh_children) {
                    context.fresh_children = false;
                    T* ptree{&context.it->second};
                    boost_iterator begin{ptree->begin()};
                    boost_iterator it{next_(ptree, begin)};
                    if (it != ptree->end()) {
                        m_stack.emplace(ptree, it, true);
                        ++m_depth;
                        return *this;
                    }
                }

                // We have previously returned the current node and its children.  Advance the base_iterator.
                ++context.it;
                context.fresh_children = true;
                next_(context.parent, context.it);
                if (context.it != context.parent->end()) {
                    return *this;
                }

                // The current base_iterator is at end.  Pop the context to ascend and continue iteration one level up.
                m_stack.pop();
                --m_depth;
            }

            return *this;
        }

        bool operator!=(const base_iterator& rhs) const
        {
            if (m_stack.empty() || rhs.m_stack.empty()) {
                return !(m_stack.empty() && rhs.m_stack.empty());
            }
            return m_stack.top().it->second != rhs.m_stack.top().it->second;
        }

        std::pair<int, T&> operator*()
        {
            if (m_stack.empty()) {
                throw Iterator_error{fmt::dereference_of_iterator_at_end};
            }
            Context& context{m_stack.top()};
            return std::pair<int, T&>{m_depth, context.it->second};
        }

    protected:
        struct Context {
            Context() = default;

            Context(T* parent_, boost_iterator it_, bool are_children_new_)
                : fresh_children(are_children_new_), it(it_), parent(parent_)
            {}

            bool fresh_children{true};
            boost_iterator it;
            T* parent{nullptr};
        };

        boost_iterator next_(T* parent, boost_iterator& it) const
        {
            while (it != parent->end()) {
                T& current{it->second};
                if (!m_filter(current)) {
                    break;
                }
                ++it;
            }
            return it;
        }

        int m_depth{0};
        bool (*m_filter)(const boost::property_tree::ptree& ptree){nullptr};
        std::stack<Context> m_stack;
    };

    base_iterator<P> begin() const
    {
        return base_iterator<P>{this->m_ptree, m_filter};
    }

    base_iterator<P> end() const
    {
        return base_iterator<P>{};
    }

    base_iterator<const boost::property_tree::ptree> cbegin() const
    {
        return base_iterator<const boost::property_tree::ptree>{this->m_ptree, m_filter};
    }

    base_iterator<const boost::property_tree::ptree> cend() const
    {
        return base_iterator<const boost::property_tree::ptree>{};
    }

    using iterator = base_iterator<boost::property_tree::ptree>;
    using const_iterator = base_iterator<const boost::property_tree::ptree>;

private:
    bool (*m_filter)(const boost::property_tree::ptree& ptree){nullptr};
    P* m_ptree{nullptr};
};

} // namespace c4lib::property_tree
