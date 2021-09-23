//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/boost_connector
//

#include <boost/connector/ops/price_greater.hpp>
#include <boost/connector/types/bid_ladder.hpp>

namespace boost::connector
{
void
bid_ladder::set(price_value price, quantity depth)
{
    auto iter =
        std::lower_bound(levels.begin(), levels.end(), price, price_greater());
    if (iter != levels.end())
    {
        if (iter->price == price)
        {
            if (depth == 0.0)
                levels.erase(iter);
            else
                iter->depth = depth;
            return;
        }
    }

    levels.insert(iter, orderbook_level { .price = price, .depth = depth });

    while (levels.size() > max_levels)
        levels.pop_back();
}
}   // namespace boost::connector