//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/boost_connector
//

#ifndef BOOST_CONNECTOR_OPS_GET_PRICE_VALUE_HPP
#define BOOST_CONNECTOR_OPS_GET_PRICE_VALUE_HPP

#include <boost/connector/ops/get_price_value.hpp>
#include <boost/connector/types/price.hpp>

namespace boost::connector
{
template < class T >
price_value
get_price_value(T const &) = delete;

}   // namespace boost::connector

#endif
