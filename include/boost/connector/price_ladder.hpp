//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#ifndef BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_PRICE_LADDER_HPP
#define BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_PRICE_LADDER_HPP

#include <boost/connector/entity/lifetime_ptr.hpp>

namespace boost::connector
{
namespace interface
{
struct price_ladder_concept;
}
/// @brief A handle to the public lifetime of a interface::pricer_ladder_concept
struct price_ladder
{
    /// @brief Construct from lifetime pointer to implementation interface
    /// @param ptr
    price_ladder(lifetime_ptr< interface::price_ladder_concept > ptr);

  private:
    lifetime_ptr< interface::price_ladder_concept > ptr_;
};
}   // namespace boost::connector
#endif   // BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_PRICE_LADDER_HPP
