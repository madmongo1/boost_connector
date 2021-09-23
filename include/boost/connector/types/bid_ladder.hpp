//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/boost_connector
//

#include <boost/connector/types/orderbook_level.hpp>

namespace boost::connector
{
/// @brief A vector of orderbook_level ordered by decreasing price
struct bid_ladder
{
    using element_type = orderbook_level;

    /// @brief Set the depth at a given price level in the ladder.
    /// @details If the price level does not exist, it is created. If the depth
    /// is equal to 0.0, the price level will be removed. If adding a price
    /// level will cause the maximum number of levels to be exceeded, the
    /// resulting lowest bid level is removed.
    /// @param price is the value of the price level.
    /// @param depth is the depth to set in the ladder.
    ///
    void
    set(price_value price, quantity depth);

    std::size_t
    size() const
    {
        return levels.size();
    }

    orderbook_level const &
    operator[](std::size_t i) const
    {
        assert(i < size());
        return levels[i];
    }

    std::vector< orderbook_level > levels;
    std::size_t                    max_levels = 1024;
};

BOOST_DESCRIBE_STRUCT(bid_ladder, (), (levels))

std::ostream &
operator<<(std::ostream &os, bid_ladder const &ladder);

}   // namespace boost::connector