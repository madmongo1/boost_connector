//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/connector/util/check_executor.hpp>

namespace boost::connector::util
{
bool
check_executor(asio::any_io_executor const &e)
{
    if (e.target_type() == typeid(asio::io_context::executor_type))
        return e.target< asio::io_context::executor_type >()->running_in_this_thread();
    if (e.target_type() == typeid(asio::strand< asio::io_context::executor_type >))
        return e.target< asio::strand< asio::io_context::executor_type > >()->running_in_this_thread();
    if (e.target_type() == typeid(asio::thread_pool::executor_type))
        return e.target< asio::thread_pool::executor_type >()->running_in_this_thread();
    if (e.target_type() == typeid(asio::strand< asio::thread_pool::executor_type >))
        return e.target< asio::strand< asio::thread_pool::executor_type > >()->running_in_this_thread();

    // return true for any executor types we don't know about.
    // This function is intended to be used in assert statements, so needn't be 100% accurate in all cases
    return true;
}

}   // namespace boost::connector::util