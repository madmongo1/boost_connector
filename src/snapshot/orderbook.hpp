//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/boost_connector
//

#ifndef BOOST_CONNECTOR_SRC_SNAPSHOT_ORDERBOOK_HPP
#define BOOST_CONNECTOR_SRC_SNAPSHOT_ORDERBOOK_HPP

#include <boost/connector/condition.hpp>
#include <boost/connector/snapshot/snapshot.hpp>
#include <boost/connector/types/bid_ladder.hpp>
#include <boost/connector/types/timestamp.hpp>

#include <memory>

namespace boost::connector
{
struct price_greater
{
    template < class L, class R >
    bool
    operator()(L const &l, R const &r) const
    {
        return get_price_value(l) > get_price_value(r);
    }
};

/// @brief A vector of orderbook_level ordered by increasing price
struct ask_ladder
{
    using vec_type       = std::vector< orderbook_level >;
    using const_iterator = vec_type::const_iterator;
    using iterator       = vec_type::iterator;

    void
                                   set(price_value price, quantity depth);
    std::vector< orderbook_level > levels;
};
BOOST_DESCRIBE_STRUCT(ask_ladder, (), (levels))
std::ostream &
operator<<(std::ostream &os, ask_ladder const &ladder);

struct orderbook
{
    /// The grouping interval of prices
    double interval;

    bid_ladder bids;

    ask_ladder asks;
};

struct orderbook_snapshot : snapshot
{
    using snapshot::snapshot;

    orderbook book;
};

}   // namespace boost::connector
#endif   // BOOST_CONNECTOR_SRC_SNAPSHOT_ORDERBOOK_HPP
