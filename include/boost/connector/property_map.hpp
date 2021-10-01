//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/boost_connector
//

#ifndef BOOST_CONNECTOR_PROPERTY_MAP_HPP
#define BOOST_CONNECTOR_PROPERTY_MAP_HPP

#include <boost/connector/property_value.hpp>

#include <ostream>
#include <unordered_map>

namespace boost::connector
{
struct property_map_layer
{
    using parent_type = std::shared_ptr< property_map_layer const >;

    property_map_layer(parent_type parent = nullptr)
    : parent_(std::move(parent))
    , map_()
    {
    }

    property_value const *
    query(std::string const &key) const
    {
        property_value const *result = nullptr;
        if (auto i = map_.find(key); i != map_.end())
            result = &i->second;
        else if (parent_)
            result = parent_->query(key);
        return result;
    }

    template < class T >
    void
    set(std::string_view name, T &&value)
    {
        static_assert(std::is_convertible_v< T &&, property_value >);
        static thread_local std::string key;
        key.assign(name.begin(), name.end());

        auto i = map_.find(key);
        if (i == map_.end())
            map_.emplace_hint(i, key, property_value(std::forward< T >(value)));
        else
            i->second = property_value(std::forward< T >(value));
    }

    parent_type parent_;
    std::
        unordered_map< std::string, property_value, boost::hash< std::string > >
            map_;
};

struct property_map
{
    property_map(std::shared_ptr< property_map_layer > impl = nullptr)
    : impl_(std::move(impl))
    {
    }

    property_value const *
    query(std::string const &key) const
    {
        return impl_->query(key);
    }

    std::shared_ptr< property_map_layer > const &
    get_implementation() const
    {
        return impl_;
    }

    std::shared_ptr< property_map_layer > impl_;
};

struct mutable_property_map
{
    mutable_property_map(property_map_layer::parent_type parent_layer = nullptr);

    template < class T >
    mutable_property_map &&
    set(std::string_view name, T &&value)
    {
        impl_->set(name, std::forward< T >(value));
        return std::move(*this);
    }

    template < class T >
    mutable_property_map &&
    set(std::string_view name, std::string_view sv)
    {
        impl_->set(name, std::string(sv.begin(), sv.end()));
        return std::move(*this);
    }

    template < class T >
    mutable_property_map &&
    set(std::string_view name, const char *sv)
    {
        impl_->set(name, std::string(sv));
        return std::move(*this);
    }

    std::shared_ptr< property_map_layer > &&
    get_implementation() &&
    {
        return std::move(impl_);
    }

    std::shared_ptr< property_map_layer > impl_;
};

inline mutable_property_map
mutate(property_map const &source_map)
{
    return mutable_property_map(source_map.get_implementation());
}

inline property_map
fix(mutable_property_map &&m)
{
    return property_map(std::move(m).get_implementation());
}

}   // namespace boost::connector
#endif   // BOOST_CONNECTOR_PROPERTY_MAP_HPP
