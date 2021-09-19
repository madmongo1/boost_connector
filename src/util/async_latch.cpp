//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#include <boost/asio/experimental/as_tuple.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/connector/config/error.hpp>
#include <boost/connector/util/async_latch.hpp>

namespace boost::connector::util
{
async_latch::async_latch(executor_type exec, bool initial_set)
: impl_ { std::move(exec),
          initial_set ? asio::steady_timer::time_point::min()
                      : asio::steady_timer::time_point::max() }
{
}

asio::awaitable< void >
async_latch::wait()
{
    if (is_set())
        co_return;

    auto [ec] = co_await impl_.async_wait(
        asio::experimental::as_tuple(asio::use_awaitable));
    if (ec == asio::error::operation_aborted)
        co_return;
    if (!ec)
        ec = asio::error::operation_aborted;
    throw system_error(ec);
}

void
async_latch::set()
{
    impl_.expires_at(asio::steady_timer::time_point::min());
}

bool
async_latch::is_set() const
{
    return impl_.expiry() == asio::steady_timer::time_point::min();
}

void
async_latch::reset()
{
    impl_.expires_at(asio::steady_timer::time_point::max());
}
}   // namespace boost::connector::util
