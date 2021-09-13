//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/connector
//

#ifndef BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_UTIL_ASYNC_CIRCULAR_BUFFER_HPP
#define BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_UTIL_ASYNC_CIRCULAR_BUFFER_HPP

#include <boost/asio/steady_timer.hpp>
#include <boost/asio/experimental/as_tuple.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/optional.hpp>

namespace boost::connector
{
template < class T, std::size_t Capacity >
struct async_circular_buffer
{
    using executor_type = asio::any_io_executor;

    async_circular_buffer(executor_type exec)
    : cv_(std::move(exec))
    , buffer_(Capacity)
    {
        cv_.expires_at(asio::steady_timer::time_point::max());
    }

    void
    clear()
    {
        buffer_.clear();
    }

    asio::awaitable< T >
    pop()
    {
        while (buffer_.size() == 0)
            co_await cv_.async_wait(asio::experimental::as_tuple(asio::use_awaitable));
        auto result = std::move(buffer_.front());
        buffer_.pop_front();
        co_return result;
    }

    void
    push(T arg)
    {
        buffer_.push_back(std::move(arg));
        cv_.cancel_one();
    }

    executor_type
    get_executor()
    {
        return cv_.get_executor();
    }

  private:
    asio::steady_timer          cv_;
    boost::circular_buffer< T > buffer_;
};

template < class T >
struct async_circular_buffer< T, 1 >
{
    using executor_type = asio::any_io_executor;

    async_circular_buffer(executor_type exec)
    : cv_(std::move(exec))
    , buffer_()
    {
        cv_.expires_at(asio::steady_timer::time_point::max());
    }

    void
    clear()
    {
        buffer_.reset();
    }

    asio::awaitable< T >
    pop()
    {
        while (!buffer_.has_value())
            co_await cv_.async_wait(asio::experimental::as_tuple(asio::use_awaitable));
        auto result = std::move(*buffer_);
        buffer_.reset();
        co_return result;
    }

    void
    push(T arg)
    {
        buffer_.emplace(std::move(arg));
        cv_.cancel_one();
    }

    executor_type
    get_executor()
    {
        return cv_.get_executor();
    }

  private:
    asio::steady_timer   cv_;
    boost::optional< T > buffer_;
};

}   // namespace boost::connector
#endif   // BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_UTIL_ASYNC_CIRCULAR_BUFFER_HPP
