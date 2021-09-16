//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/connector
//

#ifndef BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_UTIL_HASH_ANY_HPP
#define BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_UTIL_HASH_ANY_HPP

#include <boost/functional/hash.hpp>

namespace boost::connector::util
{
struct hash_any
{
    template < class T >
    std::size_t
    operator()(T const &x) const
    {
        constexpr auto op = hash< std::decay_t< T > >();
        return op(x);
    }
};

}   // namespace boost::connector::util

#endif   // BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_UTIL_HASH_ANY_HPP
