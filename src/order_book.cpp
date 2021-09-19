//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#include <boost/connector/order_book.hpp>

namespace boost::connector
{
order_book::order_book(lifetime_ptr< interface::order_book_concept > ptr)
: ptr_(std::move(ptr))
{
}

void
order_book::reset()
{
    ptr_.reset();
}

}   // namespace boost::connector
