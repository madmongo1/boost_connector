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
    asio::co_spawn(
        exec_, run(), [self = shared_from_this()](std::exception_ptr) {});
}

void
price_ladder_impl::stop()
{
    asio::dispatch(exec_,
                   [self = shared_from_this()] { self->stop_latch_.set(); });
}

price_ladder_impl::price_ladder_impl(websocket_connector connection)
: connection_(std::move(connection))
, exec_(connection_.get_executor())
{
}

std::string
price_ladder_impl::build_ident() const
{
    std::string result = "ftx::price_ladder[" + native_instrument_.channel +
                         ',' + native_instrument_.market + ']';
    return result;
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
    websocket_connector &                    connector,
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

/// @brief An asynchronous function that performs no work and continues until
/// cancelled.
template < class CompletionHandler >
auto
async_void(CompletionHandler &&token)
{
    return asio::async_compose< CompletionHandler, void(error_code) >(
        [](auto &self, auto... args)
        {
            if constexpr (sizeof...(args) == 0)
            {
                auto slot = self.get_cancellation_state().slot();

                slot.assign(
                    [self = std::move(self)](asio::cancellation_type) mutable
                    {
                        auto local_self = std::move(self);
                        local_self.get_cancellation_state().slot().clear();
                        asio::dispatch(asio::experimental::append(
                            std::move(local_self), true));
                    });
            }
            else
            {
                self.complete(asio::error::operation_aborted);
            }
        },
        token);
}
}   // namespace

auto
price_ladder_impl::monitor_subscription(frame_buffer_type &frame_buffer)
    -> asio::awaitable< monitor_event >
try
{
    using namespace asio::experimental::awaitable_operators;

    enter("price_ladder_impl::monitor_subscription");

    auto result    = monitor_event {};
    bool stop_seen = false;

    for (;;)
    {
        auto maybe_wait_stop = [stop_seen, this]
        {
            if (stop_seen)
                return async_void(asio::use_awaitable);
            else
                return stop_latch_.wait();
        };

        auto which = co_await(maybe_wait_stop() || connection_.wait_down() ||
                              frame_buffer.pop());
        switch (which.index())
        {
        case 0:
            std::cout << ident_ << " - stop signalled while subscribed\n";
            // send unsubscribe
            stop_seen = true;
            connection_.send(json::serialize(
                [this]
                {
                    auto o = json::object();
                    o.emplace("channel", native_instrument_.channel);
                    o.emplace("market", native_instrument_.market);
                    o.emplace("op", "unsubscribe");
                    return o;
                }()));
            break;

        case 1:
            std::cout << ident_ << " - connection is down\n";
            result = connection_down {};
            goto exit;

        case 2:
            auto const &response = get< json::value >(which);
            const auto &type     = response.at("type").as_string();
            if (type == "partial" || type == "update")
            {
                std::cout << ident_ << " : " << type << response.at("data")
                          << '\n';
            }
            else if (type == "error")
            {
                std::cout << ident_ << " : " << response << '\n';
                connection_.drop();
            }
            else if (type == "info")
            {
                std::cout << ident_ << " : " << response << '\n';
                connection_.drop();
            }
            else if (type == "subscribed")
            {
                std::cout << ident_ << " : " << response << '\n';
            }
            else if (type == "unsubscribed")
            {
                std::cout << ident_ << " : " << response << '\n';
                result = unsubscribe_confirmed {};
                goto exit;
            }
            else
            {
                std::cout << ident_ << " : invalid frame : " << response
                          << "\n";
                connection_.drop();
            }
        }
    }
exit:
    connection_.on(native_instrument_, nullptr);
    exit_success("price_ladder_impl::monitor_subscription");
    co_return result;
}
catch (std::exception &e)
{
    connection_.on(native_instrument_, nullptr);
    exit_exception("price_ladder_impl::monitor_subscription", e);
    throw;
}

asio::awaitable< void >
price_ladder_impl::run()
try
{
    using namespace asio::experimental::awaitable_operators;

    std::cout << ident_ << "::run()\n";

    for (;;)
    {
        auto which = co_await(stop_latch_.wait() || connection_.wait_ready());
        if (which.index() == 0)
            break;

        std::cout << ident_ << " - connection is ready\n";

        auto frame_buffer = async_circular_buffer< json::value, 1 >(
            co_await asio::this_coro::executor);

        connection_.on(native_instrument_,
                       [&frame_buffer, pident = &ident_](json::value response)
                       {
                           std::cout << *pident
                                     << "::run() -  response: " << response
                                     << '\n';
                           frame_buffer.push(std::move(response));
                       });

        // send subscription
        connection_.send(json::serialize(
            [this]
            {
                auto jo = json::object();
                jo.emplace("channel", native_instrument_.channel);
                jo.emplace("market", native_instrument_.market);
                jo.emplace("op", "subscribe");
                return jo;
            }()));

        if (auto event = co_await await_subscribe_response(frame_buffer); true)
        {
            if (holds_alternative< connection_down >(event))
            {
                connection_.on(native_instrument_, nullptr);
                continue;
            }
            else if (holds_alternative< subscribe_confirmed >(event))
            {
            }
            else if (holds_alternative< invalid_response >(event))
            {
                connection_.on(native_instrument_, nullptr);
                connection_.drop();
                continue;
            }
        }

        if (auto event = co_await monitor_subscription(frame_buffer); true)
        {
            // connection_down, invalid_response, unsubscribe_confirmed
            if (holds_alternative< connection_down >(event))
                continue;
            else if (holds_alternative< invalid_response >(event))
            {
                connection_.drop();
                continue;
            }
            else if (holds_alternative< unsubscribe_confirmed >(event))
            {
                break;
            }
        }
    }

    std::cout << "price_ladder_impl::run() - success\n";
    co_return;
}
catch (std::exception &e)
{
    std::cout << "price_ladder_impl::run() - exit with exception: " << e.what()
              << '\n';
}

auto
price_ladder_impl::await_subscribe_response(frame_buffer_type &frame_buffer)
    -> asio::awaitable< subscribe_event >
try
{
    using namespace asio::experimental::awaitable_operators;
    std::cout << ident_ << "::await_subscribe_response()";

    if (auto which = co_await(connection_.wait_down() || frame_buffer.pop());
        which.index() == 0)
    {
        std::cout << ident_
                  << "::await_subscribe_response() - exit connection_down\n";
        co_return connection_down {};
    }
    else if (which.index() == 1)
    {
        auto &response = get< json::value >(which);
        if (response.at("type") == "subscribed")
        {
            std::cout
                << ident_
                << "::await_subscribe_response() - exit subscribe_confirmed\n";
            co_return subscribe_confirmed {};
        }
        else
        {
            std::cout << ident_
                      << "::await_subscribe_response() - invalid response: "
                      << response << '\n';
            co_return invalid_response { .response = std::move(response) };
        }
    }
}
catch (std::exception &e)
{
    std::cout << ident_
              << "::await_subscribe_response() - exception: " << e.what()
              << '\n';
}

}   // namespace boost::connector::vendor::ftx