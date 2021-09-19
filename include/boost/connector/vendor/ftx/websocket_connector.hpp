//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/boost_connector
//

#ifndef BOOST_CONNECTOR_VENDOR_FTX_WEBSOCKET_CONNECTOR_HPP
#define BOOST_CONNECTOR_VENDOR_FTX_WEBSOCKET_CONNECTOR_HPP

#include <boost/connector/entity/lifetime_ptr.hpp>
#include <boost/connector/vendor/ftx/interface/ftx_websocket_connector_concept.hpp>

namespace boost::connector::vendor::ftx
{
/// A shared handle object representing the lifetime of a connection to the FTX
/// websocket API.
struct websocket_connector
{
    using executor_type = ftx_websocket_connector_concept::executor_type;
    using channel_slot  = ftx_websocket_connector_concept::channel_slot;

    websocket_connector(
        lifetime_ptr< ftx_websocket_connector_concept > lifetime);

    /// @brief Return the executor of the underlying
    /// ftx_websocket_connector_concept
    /// @pre The object must contain a valid lifetime
    /// @return executor_type
    executor_type const &
    get_executor() const;

    /// @brief Queue a text payload to be sent if the connection is up.
    /// @details The payload will be queued for send if the connection is up. If
    /// the connection drops before the payload has been sent, the payload will
    /// be dropped.
    /// @note This function must be called while executing on the executor
    /// yielded by get_executor()
    void
    send(std::string payload);

    /// @brief Wait for the connection to be fully established.
    /// @details If the connection is already fully established, return
    /// immediately.
    /// @pre must be called from the connection's executor. No marshalling takes
    /// place.
    /// @return
    asio::awaitable< void >
    wait_ready();

    /// @brief Wait for the connection to be no longer established.
    /// @details If the connection is already not fully established, return
    /// immediately.
    /// @pre must be called from the connection's executor. No marshalling takes
    /// place.
    /// @return
    asio::awaitable< void >
    wait_down();

    /// @brief Force the connection to immediately drop its connection.
    /// @pre must be called from the connection's executor. No marshalling takes
    /// place.
    void
    drop();

    /// @brief Get a reference to the underlying implementation
    /// @return ftx_websocket_connector_concept &
    /// @pre Object not empty
    ftx_websocket_connector_concept &
    get_impl();

  private:
    lifetime_ptr< ftx_websocket_connector_concept > lifetime_;
};
}   // namespace boost::connector::vendor::ftx

#endif
