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
#include <boost/connector/vendor/ftx/interface/ftx_websocket_connector.hpp>

namespace boost::connector::vendor::ftx
{
/// A shared handle object representing the lifetime of a connection to the FTX websocket API.
struct websocket_connector
{
    websocket_connector(lifetime_ptr< ftx_websocket_connector > lifetime);

  private:
    lifetime_ptr< ftx_websocket_connector > lifetime_;
};
}   // namespace boost::connector::vendor::ftx

#endif
