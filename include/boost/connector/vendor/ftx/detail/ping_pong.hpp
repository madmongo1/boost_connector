//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/connector
//

#ifndef BOOST_CONNECTOR_VENDOR_FTX_DETAIL_PING_PONG_HPP
#define BOOST_CONNECTOR_VENDOR_FTX_DETAIL_PING_PONG_HPP

#include <boost/asio/awaitable.hpp>
#include <boost/connector/util/async_circular_buffer.hpp>
#include <boost/connector/util/async_queue.hpp>
#include <boost/json/value.hpp>

namespace boost::connector::vendor::ftx::detail
{
asio::awaitable< void >
run_ping_pong(async_queue< std::string > &write_queue, async_circular_buffer< json::value, 1 > &pong_event);
}

#endif   // BOOST_CONNECTOR_SRC_VENDOR_FTX_DETAIL_PING_PONG_HPP
