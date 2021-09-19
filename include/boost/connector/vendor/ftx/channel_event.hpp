//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#ifndef BOOST_CONNECTOR_SRC_VENDOR_FTX_DETAIL_CHANNEL_EVENT_HPP
#define BOOST_CONNECTOR_SRC_VENDOR_FTX_DETAIL_CHANNEL_EVENT_HPP

#include <boost/connector/config/error.hpp>
#include <boost/json/value.hpp>
#include <boost/variant2/variant.hpp>

namespace boost::connector::vendor::ftx
{
struct connection_up
{
};

struct connection_down
{
};

/// @brief indicates an event on a channel
struct channel_event
{
    channel_event(connection_up e)
    : var_(e)
    {
    }

    channel_event(connection_down e)
    : var_(e)
    {
    }

    channel_event(json::value v)
    : var_(std::move(v))
    {
    }

    channel_event(error_code ec)
    : var_(ec)
    {
    }

    bool
    is_down() const
    {
        return holds_alternative< connection_down >(var_);
    }

    bool
    is_error() const
    {
        return holds_alternative< error_code >(var_);
    }

    bool
    is_up() const
    {
        return holds_alternative< connection_up >(var_);
    }

    bool
    is_response() const
    {
        return holds_alternative< json::value >(var_);
    }

    /// @brief Test if the event is a response of type "subscribed"
    /// @return boolean.
    /// @throws never throws. A malformed json value will result in a false
    /// return value.
    bool
    is_subscribed() const
    {
        auto result = false;

        if (auto pval = get_if< json::value >(&var_))
            if (auto pobject = pval->if_object())
                if (auto ptype = pobject->if_contains("type"))
                    if (auto pstr = ptype->if_string())
                        if (*pstr == "subscribed")
                            result = true;

        return result;
    }

    json::value &
    get_response()
    {
        return get< json::value >(var_);
    }

    json::value const &
    get_response() const
    {
        return get< json::value >(var_);
    }

    error_code const &
    get_error() const
    {
        return get< error_code >(var_);
    }

  private:
    using var_type = variant2::
        variant< connection_down, connection_up, json::value, error_code >;
    var_type var_;
};

}   // namespace boost::connector::vendor::ftx

#endif   // BOOST_CONNECTOR_SRC_VENDOR_FTX_DETAIL_CHANNEL_EVENT_HPP
