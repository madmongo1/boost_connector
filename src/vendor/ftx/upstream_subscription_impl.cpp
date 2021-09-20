//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/connector/vendor/ftx/upstream_subscription_impl.hpp>
#include <boost/json.hpp>

#include <iostream>

namespace boost::connector::vendor::ftx
{
upstream_subscription_impl::upstream_subscription_impl(
    websocket_connector connection,
    channel_market_pair topic)
: connection_(std::move(connection))
, topic_(std::move(topic))
, ident_(build_ident())
{
}

void
upstream_subscription_impl::start()
{
    // spawn the run coroutine on the connector's executor while holding on to
    // the private lifetime of the implementation
    asio::co_spawn(connection_.get_executor(),
                   run(),
                   [self = shared_from_this()](std::exception_ptr) {});
}

void
upstream_subscription_impl::stop()
{
    asio::dispatch(connection_.get_executor(),
                   [self = shared_from_this()] { self->stop_latch_.set(); });
}

std::string
upstream_subscription_impl::build_ident() const
{
    std::ostringstream ss;
    ss << "ftx::upstream_subscription[" << topic() << ']';
    return std::move(ss).str();
}

asio::awaitable< void >
upstream_subscription_impl::run()
try
{
    using namespace asio::experimental::awaitable_operators;

    std::cout << ident_ << "::run()\n";

    auto &connection_impl = connection_.get_impl();

    auto  iter  = connection_impl.locate_channel(topic());
    auto &state = iter->second;

    // wait for channel state to become unacquired

    auto closed_latch = util::async_latch(co_await asio::this_coro::executor);
    auto sm           = state_machine { .algo         = *this,
                              .connection   = connection_impl,
                              .csiter       = iter,
                              .closed_latch = closed_latch };

    while (state.acquired)
    {
        auto which = co_await(this->stop_latch_.wait() ||
                              state.cv.async_wait(asio::experimental::as_tuple(
                                  asio::use_awaitable)));
        if (which.index() == 0)
        {
            std::cout << "price_ladder_impl::run() - stopped while waiting for "
                         "acquisition\n";
            co_return;
        }
        else
            break;
    }

    std::cout << "price_ladder_impl::run() - acquired channel\n";
    state.acquired = true;
    state.on_event = [&sm](channel_event const &e) { sm.process_event(e); };
    if (connection_impl.is_up())
        sm.process_event(connection_up());

    std::cout << "price_ladder_impl::run() - awaiting stop_latch\n";
    co_await stop_latch_.wait();
    sm.close();

    std::cout << "price_ladder_impl::run() - awaiting closed_latch\n";
    co_await closed_latch.wait();
    connection_impl.release_channel(iter);

    std::cout << "price_ladder_impl::run() - success\n";
    co_return;
}
catch (std::exception &e)
{
    std::cout << "price_ladder_impl::run() - exit with exception: " << e.what()
              << '\n';
}

void
upstream_subscription_impl::on_down()
{
}

void
upstream_subscription_impl::on_up()
{
}

void
upstream_subscription_impl::on_data(const json::object &data)
{
}

void
upstream_subscription_impl::state_machine::enter_closed_state()
{
    state = closed;
    closed_latch.set();
}

void
upstream_subscription_impl::state_machine::enter_closing_state()
{
    state = closing;
    connection.send(make_subscribe_message("unsubscribe"));
}

void
upstream_subscription_impl::state_machine::close()
{
    if (std::exchange(close_requested, true))
        return;

    auto &cs = csiter->second;
    switch (state)
    {
    case down:
        enter_closed_state();
        break;

    case open:
        enter_closing_state();
        break;

    case opening:
    case closing:
    case closed:
        break;
    }
}

void
upstream_subscription_impl::state_machine::process_event(const channel_event &e)
{
    auto &cs = csiter->second;
    switch (state)
    {
    case down:
        process_down_event(e);
        break;
    case opening:
        process_opening_event(e);
        break;
    case open:
        process_open_event(e);
        break;
    case closing:
        process_closing_event(e);
        break;
    case closed:
        process_closed_event(e);
        break;
    }
}

void
upstream_subscription_impl::state_machine::process_down_event(
    const channel_event &e)
{
    if (e.is_up())
    {
        assert(connection.is_up());
        state = opening;
        std::cout << "pricer_ladder_impl[" << csiter->first << "]: UP\n";
        connection.send(make_subscribe_message("subscribe"));
    }
    else
    {
        std::cout << "pricer_ladder_impl[" << csiter->first
                  << "]: invalid event when DOWN\n";
        connection.drop();
    }
}

void
upstream_subscription_impl::state_machine::process_opening_event(
    const channel_event &e)
{
    if (e.is_down())
    {
        std::cout << "pricer_ladder_impl[" << csiter->first << "]: DOWN\n";
        state = down;
    }
    else if (e.is_subscribed())
    {
        std::cout << "pricer_ladder_impl[" << csiter->first << "]: OPEN\n";
        if (close_requested)
            enter_closing_state();
        else
            state = open;
    }
    else
    {
        std::cout << "pricer_ladder_impl[" << csiter->first
                  << "]: invalid event when OPENING\n";
        connection.drop();
    }
}

void
upstream_subscription_impl::state_machine::handle_info(const json::object &o)
{
    std::cout << "pricer_ladder_impl[" << csiter->first << "]: info: " << o
              << "\n";
    if (auto pcode = o.if_contains("code"))
        if (auto pi = pcode->if_int64())
            if (*pi == 20001)
                connection.drop();
}

void
upstream_subscription_impl::state_machine::handle_partial(const json::object &o)
{
    std::cout << "pricer_ladder_impl[" << csiter->first << "]: partial: " << o
              << "\n";
}

void
upstream_subscription_impl::state_machine::handle_update(const json::object &o)
{
    std::cout << "pricer_ladder_impl[" << csiter->first << "]: update: " << o
              << "\n";
}

void
upstream_subscription_impl::state_machine::handle_error(const json::object &o)
{
    std::cout << "pricer_ladder_impl[" << csiter->first << "]: error: " << o
              << "\n";
    connection.drop();
}

void
upstream_subscription_impl::state_machine::process_open_event(
    const channel_event &e)
{
    if (e.is_down())
    {
        std::cout << "pricer_ladder_impl[" << csiter->first << "]: DOWN\n";
        state = down;
    }
    else if (e.is_up())
    {
        std::cout << "pricer_ladder_impl[" << csiter->first
                  << "]: invalid event when OPEN\n";
    }
    else
    {
        try
        {
            auto &o    = e.get_response().as_object();
            auto &type = o.at("type").as_string();
            if (type == "info")
                handle_info(o);
            else if (type == "partial")
                handle_partial(o);
            else if (type == "update")
                handle_update(o);
            else
                handle_error(o);
        }
        catch (std::exception &e)
        {
            std::cout << "pricer_ladder_impl[" << csiter->first
                      << "]: error decoding response when OPEN\n";
            connection.drop();
        }
    }
}

void
upstream_subscription_impl::state_machine::process_closing_event(
    const channel_event &e)
{
    if (e.is_down())
    {
        std::cout << "pricer_ladder_impl[" << csiter->first
                  << "]: DOWN -> CLOSED\n";
        enter_closed_state();
    }
    else if (e.is_up())
    {
        std::cout << "pricer_ladder_impl[" << csiter->first
                  << "]: invalid event when OPEN\n";
    }
    else
    {
        try
        {
            auto &o    = e.get_response().as_object();
            auto &type = o.at("type").as_string();
            if (type == "info")
                handle_info(o);
            else if (type == "partial")
                handle_partial(o);
            else if (type == "update")
                handle_update(o);
            else if (type == "unsubscribed")
                enter_closed_state();
            else
                handle_error(o);
        }
        catch (std::exception &e)
        {
            std::cout << "pricer_ladder_impl[" << csiter->first
                      << "]: error decoding response when CLOSING\n";
            connection.drop();
        }
    }
}

void
upstream_subscription_impl::state_machine::process_closed_event(
    const channel_event &e)
{
    assert(!"logic error - event seen when closed");
}

std::string
upstream_subscription_impl::state_machine::make_subscribe_message(
    boost::string_view action) const
{
    auto jo = json::object();
    jo.emplace("channel", csiter->first.channel);
    jo.emplace("market", csiter->first.market);
    jo.emplace("op", action);
    return boost::json::serialize(jo);
}
}   // namespace boost::connector::vendor::ftx
