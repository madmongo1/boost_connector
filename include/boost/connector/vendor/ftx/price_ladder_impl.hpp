//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#ifndef BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_VENDOR_FTX_IMPLEMENTATION_PRICE_LADDER_IMPL_HPP
#define BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_VENDOR_FTX_IMPLEMENTATION_PRICE_LADDER_IMPL_HPP

#include <boost/connector/interface/order_book_concept.hpp>
#include <boost/connector/util/async_latch.hpp>
#include <boost/connector/vendor/ftx/channel_market_pair.hpp>
#include <boost/connector/vendor/ftx/upstream_subscription_impl.hpp>
#include <boost/connector/vendor/ftx/websocket_connector.hpp>

namespace boost::connector::vendor::ftx
{
/// @brief Implement the concept of a price ladder for FTX
struct price_ladder_impl final
: interface::order_book_concept
, upstream_subscription_impl
, std::enable_shared_from_this< price_ladder_impl >
{
    /// @brief Construct a pricer_ladder_impl
    /// @param connection is a websocket_connector handle containing a valid
    /// entity lifetime
    /// @param market is the FTX-specific market name
    price_ladder_impl(websocket_connector connection, std::string market);

    // interface::price_ladder_concept
    void
    start() override;

    void
    stop() override;
};

}   // namespace boost::connector::vendor::ftx
#endif   // BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_VENDOR_FTX_IMPLEMENTATION_PRICE_LADDER_IMPL_HPP
