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

#include <boost/connector/interface/price_ladder_concept.hpp>
#include <boost/connector/util/async_latch.hpp>
#include <boost/connector/vendor/ftx/channel_market_pair.hpp>
#include <boost/connector/vendor/ftx/websocket_connector.hpp>
#include <boost/variant2/variant.hpp>

namespace boost::connector::vendor::ftx
{
/// @brief Implement the concept of a price ladder for FTX
struct price_ladder_impl
: interface::price_ladder_concept
, std::enable_shared_from_this< price_ladder_impl >
{
    price_ladder_impl(websocket_connector connection);

    // interface::price_ladder_concept
    void
    start() override;

    void
    stop() override;

  private:
    using frame_buffer_type = async_circular_buffer< json::value, 1 >;

    asio::awaitable< void >
    run();

    struct connection_down
    {
    };
    struct subscribe_confirmed
    {
    };
    struct unsubscribe_confirmed
    {
    };
    struct invalid_response
    {
        json::value response;
    };
    using subscribe_event = variant2::
        variant< connection_down, subscribe_confirmed, invalid_response >;

    asio::awaitable< subscribe_event >
    await_subscribe_response(frame_buffer_type &frame_buffer);

    using monitor_event = variant2::
        variant< connection_down, invalid_response, unsubscribe_confirmed >;

    asio::awaitable< monitor_event >
    monitor_subscription(frame_buffer_type &frame_buffer);

    std::string
    build_ident() const;

  private:
    websocket_connector   connection_;
    asio::any_io_executor exec_;
    channel_market_pair   native_instrument_ = { .channel = "orderbook",
                                               .market  = "BTC/USD" };
    std::string           ident_             = build_ident();

    // Set this latch to instruct the internal master coroutine to stop
    util::async_latch stop_latch_ { exec_ };
};

}   // namespace boost::connector::vendor::ftx
#endif   // BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_VENDOR_FTX_IMPLEMENTATION_PRICE_LADDER_IMPL_HPP
