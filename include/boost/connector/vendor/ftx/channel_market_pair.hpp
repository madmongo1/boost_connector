//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#ifndef BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_VENDOR_FTX_CHANNEL_MARKET_PAIR_HPP
#define BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_VENDOR_FTX_CHANNEL_MARKET_PAIR_HPP

#include <boost/connector/vendor/ftx/describe_operators.hpp>
#include <boost/connector/vendor/ftx/equality.hpp>

namespace boost::connector::vendor::ftx
{
struct channel_market_pair
{
    std::string channel;
    std::string market;
};

BOOST_DESCRIBE_STRUCT(channel_market_pair, (), (channel, market));

}   // namespace boost::connector::vendor::ftx
#endif   // BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_VENDOR_FTX_CHANNEL_MARKET_PAIR_HPP
