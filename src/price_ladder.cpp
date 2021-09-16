//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#include <boost/connector/price_ladder.hpp>

namespace boost::connector
{
price_ladder::price_ladder(lifetime_ptr< interface::price_ladder_concept > ptr)
: ptr_(std::move(ptr))
{
}

}   // namespace boost::connector
