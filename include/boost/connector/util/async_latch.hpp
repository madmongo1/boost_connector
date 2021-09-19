//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#ifndef BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_UTIL_ASYNC_LATCH_HPP
#define BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_UTIL_ASYNC_LATCH_HPP

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/steady_timer.hpp>

namespace boost::connector::util
{
struct async_latch
{
    using executor_type = asio::any_io_executor;

    async_latch(executor_type exec, bool initial_set = false);

    asio::awaitable< void >
    wait();

    void
    set();

    bool
    is_set() const;

    void
    reset();

  private:
    asio::steady_timer impl_;
};

}   // namespace boost::connector::util

#endif   // BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_UTIL_ASYNC_LATCH_HPP
