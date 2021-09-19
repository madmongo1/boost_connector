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

/*
auto
price_ladder_impl::monitor_subscription()
    -> asio::awaitable< void >
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
                return util::async_void(asio::use_awaitable);
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
    exit_success("price_ladder_impl::monitor_subscription");
    co_return result;
}
catch (std::exception &e)
{
    exit_exception("price_ladder_impl::monitor_subscription", e);
    throw;
}
 */

namespace
{
bool                    stop_requested_ = false;
std::function< void() > on_stop_;

void
handle_stop()
{
    if (!std::exchange(stop_requested_, true))
        if (on_stop_)
        {
            auto f = std::exchange(on_stop_, nullptr);
            f();
        }
}

}   // namespace

namespace
{
struct state_machine
{
    void
    enter_closed_state()
    {
        state = closed;
        closed_latch.set();
    }

    void
    enter_closing_state()
    {
        state = closing;
        connection.send(make_subscribe_message("unsubscribe"));
    }

    void
    close()
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
    process_event(channel_event const &e)
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
    process_down_event(channel_event const &e)
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
    process_opening_event(channel_event const &e)
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
    handle_info(json::object const &o)
    {
        std::cout << "pricer_ladder_impl[" << csiter->first << "]: info: " << o
                  << "\n";
        if (auto pcode = o.if_contains("code"))
            if (auto pi = pcode->if_int64())
                if (*pi == 20001)
                    connection.drop();
    }

    void
    handle_partial(json::object const &o)
    {
        std::cout << "pricer_ladder_impl[" << csiter->first
                  << "]: partial: " << o << "\n";
    }
    void
    handle_update(json::object const &o)
    {
        std::cout << "pricer_ladder_impl[" << csiter->first
                  << "]: update: " << o << "\n";
    }

    void
    handle_error(json::object const &o)
    {
        std::cout << "pricer_ladder_impl[" << csiter->first << "]: error: " << o
                  << "\n";
        connection.drop();
    }

    void
    process_open_event(channel_event const &e)
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
    process_closing_event(channel_event const &e)
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
    process_closed_event(channel_event const &e)
    {
        assert(!"logic error - event seen when closed");
    }

    std::string
    make_subscribe_message(boost::string_view action) const
    {
        auto jo = json::object();
        jo.emplace("channel", csiter->first.channel);
        jo.emplace("market", csiter->first.market);
        jo.emplace("op", action);
        return boost::json::serialize(jo);
    }

    price_ladder_impl               &ladder_algo;
    ftx_websocket_connector_concept &connection;
    channel_state_map::iterator      csiter;
    util::async_latch               &closed_latch;
    enum state_type
    {
        down,
        opening,
        open,
        closing,
        closed,
    } state = down;

    bool close_requested = false;
};
}   // namespace

asio::awaitable< void >
price_ladder_impl::run()
try
{
    using namespace asio::experimental::awaitable_operators;

    std::cout << ident_ << "::run()\n";

    auto &connection_impl = connection_.get_impl();

    auto  iter  = connection_impl.locate_channel(native_instrument_);
    auto &state = iter->second;

    // wait for channel state to become unacquired

    auto closed_latch = util::async_latch(co_await asio::this_coro::executor);
    auto sm           = state_machine { .ladder_algo  = *this,
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

}   // namespace boost::connector::vendor::ftx