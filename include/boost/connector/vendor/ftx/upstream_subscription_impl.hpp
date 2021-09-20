//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#ifndef BOOST_CONNECTOR_SRC_VENDOR_FTX_UPSTREAM_SUBSCRIPTION_IMPL_HPP
#define BOOST_CONNECTOR_SRC_VENDOR_FTX_UPSTREAM_SUBSCRIPTION_IMPL_HPP

#include <boost/connector/vendor/ftx/channel_market_pair.hpp>
#include <boost/connector/vendor/ftx/websocket_connector.hpp>

namespace boost::connector::vendor::ftx
{
struct upstream_subscription_impl
: std::enable_shared_from_this< upstream_subscription_impl >

{
    upstream_subscription_impl(websocket_connector connection,
                               channel_market_pair topic);

    // provide start and stop for make_lifetime_ptr<>
    void
    start();

    void
    stop();

    inline channel_market_pair const &
    topic() const;

  protected:
    std::string
    build_ident() const;

    asio::awaitable< void >
    run();

    virtual void
    on_down();

    virtual void
    on_up();

    virtual void
    on_data(json::object const &data);

  private:
    struct state_machine
    {
        void
        enter_closed_state();

        void
        enter_closing_state();

        void
        close();

        void
        process_event(channel_event const &e);

        void
        process_down_event(channel_event const &e);

        void
        process_opening_event(channel_event const &e);

        void
        handle_info(json::object const &o);

        void
        handle_partial(json::object const &o);

        void
        handle_update(json::object const &o);

        void
        handle_error(json::object const &o);

        void
        process_open_event(channel_event const &e);

        void
        process_closing_event(channel_event const &e);

        void
        process_closed_event(channel_event const &e);

        std::string
        make_subscribe_message(boost::string_view action) const;

        upstream_subscription_impl &     algo;
        ftx_websocket_connector_concept &connection;
        channel_state_map::iterator      csiter;
        util::async_latch &              closed_latch;
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

  protected:
    websocket_connector connection_;
    channel_market_pair topic_;

    // Set this latch to instruct the internal master coroutine to stop
    util::async_latch stop_latch_ { connection_.get_executor() };

    std::string const ident_;
};

}   // namespace boost::connector::vendor::ftx

namespace boost::connector::vendor::ftx
{
channel_market_pair const &
upstream_subscription_impl::topic() const
{
    return topic_;
}
}   // namespace boost::connector::vendor::ftx

#endif   // BOOST_CONNECTOR_SRC_VENDOR_FTX_UPSTREAM_SUBSCRIPTION_IMPL_HPP
