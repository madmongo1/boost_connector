//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/experimental/as_tuple.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/connector/vendor/ftx/channel_state.hpp>
#include <boost/connector/vendor/ftx/interface/ftx_websocket_connector_concept.hpp>
#include <boost/json/serialize.hpp>

namespace boost::connector::vendor::ftx
{

}   // namespace boost::connector::vendor::ftx