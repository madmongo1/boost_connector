//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/boost_connector
//

#include <boost/connector/dependency/dependency_map.hpp>

namespace boost::connector
{
std::any const *
dependency_map_impl::query(std::string const &name) const
{
    std::any const *result = nullptr;
    auto            i      = map_.find(name);
    if (i != map_.end())
        result = std::addressof(i->second);
    return result;
}

void
dependency_map_impl::set(std::string name, std::any value)
{
    auto i = map_.find(name);
    if (i == map_.end())
        map_.emplace(std::move(name), std::move(value));
    else
        i->second = std::move(value);
}

dependency_map::dependency_map()
: impl_(std::make_shared< dependency_map_impl >())
{
}

dependency_map::dependency_map(std::shared_ptr< dependency_map_impl const > impl) noexcept
: impl_(std::move(impl))
{
}

mutable_dependency_map::mutable_dependency_map()
: impl_(std::make_shared< dependency_map_impl >())
{
}

mutable_dependency_map::mutable_dependency_map(std::shared_ptr< dependency_map_impl > impl) noexcept
: impl_(std::move(impl))
{
}

dependency_map
fix(mutable_dependency_map &&source)
{
    return dependency_map(std::move(source.impl_));
}

mutable_dependency_map
clone(dependency_map const &source)
{
    return mutable_dependency_map(std::make_shared< dependency_map_impl >(*source.impl_));
}

}   // namespace boost::connector