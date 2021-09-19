//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#include <boost/connector/vendor/ftx/channel_event.hpp>
#include <boost/json/parse.hpp>
#include <catch2/catch.hpp>

TEST_CASE("vendor::ftx::channel_event is_down")
{
    using namespace boost::connector;

    auto e1 = vendor::ftx::channel_event(vendor::ftx::connection_down());
    CHECK(e1.is_down());
    CHECK(!e1.is_response());
}

TEST_CASE("vendor::ftx::channel_event is_response")
{
    using namespace boost::connector;

    auto e1 = vendor::ftx::channel_event(boost::json::parse("10"));
    CHECK(!e1.is_down());
    REQUIRE(e1.is_response());
    auto &j = e1.get_response();
    REQUIRE(j.is_int64());
    CHECK(j.as_int64() == 10);
}