//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#ifndef BOOST_CONNECTOR_SRC_VENDOR_FTX_CHANNEL_STATE_HPP
#define BOOST_CONNECTOR_SRC_VENDOR_FTX_CHANNEL_STATE_HPP

#include <boost/connector/vendor/ftx/channel_event.hpp>
#include <boost/connector/vendor/ftx/channel_market_pair.hpp>
#include <boost/functional/hash.hpp>
#include <boost/asio/steady_timer.hpp>

#include <functional>
#include <unordered_map>

namespace boost::connector::vendor::ftx
{
struct ftx_websocket_connector_concept;

/// @brief A channel_state is owned by the producer of events.
struct channel_state
{
    asio::steady_timer cv;

    /// @brief Maintain a pointer to the consumer state, allowing us to post
    /// events to it.
    std::function< void(channel_event const&) > on_event = nullptr;

    // Data owned by the producer
    int  interest = 0;   /// The number of consumers interested in this producer
    bool acquired = false;   /// True if a consumer has acquired the state
};

using channel_state_map =
    std::unordered_map< channel_market_pair,
                        channel_state,
                        boost::hash< channel_market_pair > >;

/*
 *
 * acquire_interest(index)
 * {
 *   auto caller = asio::this_coro::executor;
 *   auto my_executor = get_executor();
 *
 *   auto op = [](auto self, channel_market_pair const& index) {
 *      // we are now on the connection's executor
 *      auto& channel_state_ptr = channels_[index];
 *      if(!channel_state_ptr
 *      channel_state->interest += 1;
 *      co_await channel_state->await_free();
 *      if (is_up())
 *          send(subscribe_message(index, "subscribe"));
 *
 *   check:
 *      auto iter = channels_.find(index);
 *      if (iter != channels_.end())
 *      {
 *        co_await
 *   };
 *
 *   if (caller != get_executor)
 *      return co_spawn(my_executor, op(shared_from_this()), use_awaitable);
 *   else
 *      return op(shared_from_this());
 *
 * }
 *

 */

}   // namespace boost::connector::vendor::ftx

#endif   // BOOST_CONNECTOR_SRC_VENDOR_FTX_CHANNEL_STATE_HPP
