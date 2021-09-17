//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/boost_connector
//

#ifndef BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_VENDOR_FTX_DESCRIBE_OPERATORS_HPP
#define BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_VENDOR_FTX_DESCRIBE_OPERATORS_HPP

#include <boost/connector/util/equals.hpp>
#include <boost/connector/util/hash_value.hpp>
#include <boost/connector/util/ostream.hpp>

namespace boost::connector::vendor::ftx
{
using ::boost::connector::hash_value;
using ::boost::connector::operator==;
using ::boost::connector::operator<<;
}   // namespace boost::connector::vendor::ftx
#endif   // BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_VENDOR_FTX_DESCRIBE_OPERATORS_HPP
