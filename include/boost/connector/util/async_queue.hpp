//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/boost_connector
//

#ifndef BOOST_CONNECTOR__UTIL__ASYNC_QUEUE_HPP
#define BOOST_CONNECTOR__UTIL__ASYNC_QUEUE_HPP

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/experimental/as_tuple.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <deque>

namespace boost::connector
{
template < class T >
struct async_queue
{
    using executor_type = asio::any_io_executor;

    async_queue(executor_type exec)
    : cv_(std::move(exec))
    {
        cv_.expires_at(asio::steady_timer::time_point::max());
    }

    void
    clear()
    {
        queue_.clear();
    }

    asio::awaitable< T >
    pop()
    {
        while (queue_.size() == 0)
            co_await cv_.async_wait(asio::experimental::as_tuple(asio::use_awaitable));
        auto result = std::move(queue_.front());
        queue_.pop_front();
        co_return result;
    }

    void
    push(T arg)
    {
        queue_.push_back(std::move(arg));
        cv_.cancel_one();
    }

    executor_type
    get_executor()
    {
        return cv_.get_executor();
    }

  private:
    asio::steady_timer cv_;
    std::deque< T >    queue_;
};


}   // namespace boost::connector
#endif   // BOOST_CONNECTOR_SRC_UTIL_ASYNC_QUEUE_HPP
