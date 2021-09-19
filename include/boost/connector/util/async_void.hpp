//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#ifndef BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_UTIL_ASYNC_VOID_HPP
#define BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_UTIL_ASYNC_VOID_HPP

#include <boost/asio/compose.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/error.hpp>

namespace boost::connector::util
{
/// @brief An asynchronous function that performs no work and continues until
/// cancelled.
template < class CompletionHandler >
auto
async_void(CompletionHandler &&token)
{
    return asio::async_compose< CompletionHandler, void(error_code) >(
        [](auto &self, auto... args)
        {
            if constexpr (sizeof...(args) == 0)
            {
                auto slot = self.get_cancellation_state().slot();

                slot.assign(
                    [self = std::move(self)](asio::cancellation_type) mutable
                    {
                        auto local_self = std::move(self);
                        local_self.get_cancellation_state().slot().clear();
                        asio::dispatch(asio::experimental::append(
                            std::move(local_self), true));
                    });
            }
            else
            {
                self.complete(asio::error::operation_aborted);
            }
        },
        token);
}

}   // namespace boost::connector::util
#endif   // BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_UTIL_ASYNC_VOID_HPP
