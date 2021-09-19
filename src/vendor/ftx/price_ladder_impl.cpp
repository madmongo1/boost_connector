//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#include "boost/connector/vendor/ftx/price_ladder_impl.hpp"

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/experimental/append.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/connector/config/error.hpp>
#include <boost/connector/util/async_circular_buffer.hpp>
#include <boost/connector/util/async_void.hpp>
#include <boost/connector/vendor/ftx/price_ladder_impl.hpp>
#include <boost/json.hpp>

#include <iostream>

namespace boost::connector::vendor::ftx
{
void
price_ladder_impl::start()
{
    // spawn the run coroutine on the connector's executor while holding on to
    // the private lifetime of the implementation
    asio::co_spawn(connection_.get_executor(),
                   run(),
                   [self = shared_from_this()](std::exception_ptr) {});
}

void
price_ladder_impl::stop()
{
    asio::dispatch(connection_.get_executor(),
                   [self = shared_from_this()] { self->stop_latch_.set(); });
}

price_ladder_impl::price_ladder_impl(websocket_connector connection,
                                     std::string         market)
: upstream_subscription_impl(
      std::move(connection),
      channel_market_pair { .channel = "orderbook",
                            .market  = std::move(market) })
{
}

namespace
{
void
enter(std::string_view f)
{
    std::cout << f << " - enter\n";
}
void
exit_success(std::string_view f)
{
    std::cout << f << " - exit success\n";
}
void
exit_exception(std::string_view f, std::exception &e)
{
    std::cout << f << " - exit exception: " << e.what() << "\n";
}

asio::awaitable< void >
wait_for_unsub_confirmation(
    websocket_connector                     &connector,
    async_circular_buffer< json::value, 1 > &frame_buffer)
try
{
    using namespace asio::experimental::awaitable_operators;
    enter("wait_for_unsub_confirmation");

    for (;;)
    {
        auto which = co_await(connector.wait_down() || frame_buffer.pop());
        switch (which.index())
        {
        case 0:
            co_return;

        case 1:
            auto const &frame = get< json::value >(which);
            auto const &type  = frame.at("type").as_string();
            if (type == "unsubscribed")
                co_return;
        }
    }
    exit_success("wait_for_unsub_confirmation");
}
catch (std::exception &e)
{
    exit_exception("wait_for_unsub_confirmation", e);
}

}   // namespace

}   // namespace boost::connector::vendor::ftx