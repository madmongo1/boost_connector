//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/boost_connector
//

#ifndef BOOST_CONNECTOR_OPS_PRICE_GREATER_HPP
#define BOOST_CONNECTOR_OPS_PRICE_GREATER_HPP

#include <boost/connector/ops/get_price_value.hpp>

namespace boost::connector
{
/// @brief A general purpose function object to compare the price levels of two
/// objects of any compatible type
struct price_greater
{
    /// @brief Call operator
    /// @tparam L is the type of the left hand argument.
    /// @tparam R is the type of the right hand argument
    /// @param l is the left hand side of the comparison
    /// @param r is the right hand side of the comparison
    /// @note the following expressions must be well formed:
    /// @code {.cpp}
    /// price_value get_price_value(L const& l);
    /// price_value get_price_value(R const& r);
    /// @endcode

    template < class L, class R >
    bool
    operator()(L const &l, R const &r) const
    {
        return get_price_value(l) > get_price_value(r);
    }
};

}   // namespace boost::connector

#endif
