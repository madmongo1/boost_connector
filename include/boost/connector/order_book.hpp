//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#ifndef BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_ORDER_BOOK_HPP
#define BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_ORDER_BOOK_HPP

#include <boost/connector/entity/lifetime_ptr.hpp>

namespace boost::connector
{
namespace interface
{
struct order_book_concept;
}
/// @brief A handle to the public lifetime of a interface::pricer_ladder_concept
struct order_book
{
    /// @brief Construct from lifetime pointer to implementation interface
    /// @param ptr
    order_book(lifetime_ptr< interface::order_book_concept > ptr);

    void
    reset();

  private:
    lifetime_ptr< interface::order_book_concept > ptr_;
};
}   // namespace boost::connector
#endif   // BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_ORDER_BOOK_HPP
