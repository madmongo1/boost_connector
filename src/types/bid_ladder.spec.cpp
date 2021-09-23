#include <boost/connector/types/bid_ladder.hpp>
#include <catch2/catch.hpp>

using namespace boost::connector;

TEST_CASE("boost::connector::bid_ladder")
{
    auto bids = bid_ladder();
    bids.set(10000, 100);
    bids.set(9999, 200);
    bids.set(10001, 25);

    CHECK(bids.size() == 3);
    REQUIRE(bids.size() == 3);
    CHECK(bids[0] == orderbook_level { .price = 10001, .depth = 25 });
    CHECK(bids[1] == orderbook_level { .price = 10000, .depth = 100 });
    CHECK(bids[2] == orderbook_level { .price = 9999, .depth = 200 });

    bids.set(10000, 200);
    REQUIRE(bids.size() == 3);
    CHECK(bids[0] == orderbook_level { .price = 10001, .depth = 25 });
    CHECK(bids[1] == orderbook_level { .price = 10000, .depth = 200 });
    CHECK(bids[2] == orderbook_level { .price = 9999, .depth = 200 });

    bids.set(9999, 150);
    REQUIRE(bids.size() == 3);
    CHECK(bids[0] == orderbook_level { .price = 10001, .depth = 25 });
    CHECK(bids[1] == orderbook_level { .price = 10000, .depth = 200 });
    CHECK(bids[2] == orderbook_level { .price = 9999, .depth = 150 });

    bids.set(9999, 0);
    REQUIRE(bids.size() == 2);
    CHECK(bids[0] == orderbook_level { .price = 10001, .depth = 25 });
    CHECK(bids[1] == orderbook_level { .price = 10000, .depth = 200 });

    bids.max_levels = 2;
    bids.set(9999, 150);
    REQUIRE(bids.size() == 2);
    CHECK(bids[0] == orderbook_level { .price = 10001, .depth = 25 });
    CHECK(bids[1] == orderbook_level { .price = 10000, .depth = 200 });

    bids.set(10002, 150);
    REQUIRE(bids.size() == 2);
    CHECK(bids[0] == orderbook_level { .price = 10002, .depth = 150 });
    CHECK(bids[1] == orderbook_level { .price = 10001, .depth = 25 });
}