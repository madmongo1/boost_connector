//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/boost_connector
//

#ifndef BOOST_CONNECTOR_TYPES_ORDERBOOK_LEVEL_HPP
#define BOOST_CONNECTOR_TYPES_ORDERBOOK_LEVEL_HPP

#include <boost/connector/types/price.hpp>
#include <boost/connector/types/quantity.hpp>
#include <boost/connector/util/describe_operators.hpp>

namespace boost::connector
{
struct orderbook_level
{
    price_value price;
    quantity    depth;
};
BOOST_DESCRIBE_STRUCT(orderbook_level, (), (price, depth))

std::ostream &
operator<<(std::ostream &os, orderbook_level const &level);

inline price_value
get_price_value(orderbook_level const &ol)
{
    return ol.price;
}

}   // namespace boost::connector

#endif
