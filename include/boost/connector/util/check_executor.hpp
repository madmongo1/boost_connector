//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#ifndef BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_UTIL_CHECK_EXECUTOR_HPP
#define BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_UTIL_CHECK_EXECUTOR_HPP

#include <boost/asio/any_io_executor.hpp>

namespace boost::connector::util
{
/// @brief Provide a best-effort check that code is running under the correct execuctor
/// @param e is the executor to check
/// @return true if running under the correct executor. Also true if the underlying executor type is not recognised.
/// @note if the underlying executor is not one of a known set of types, the function will return true, allowing it to
/// be used in `assert` clauses.
bool
check_executor(asio::any_io_executor const &e);

}   // namespace boost::connector::util
#endif   // BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_UTIL_CHECK_EXECUTOR_HPP
