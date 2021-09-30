//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/boost_connector
//

#include <boost/connector/property_map.hpp>
#include <catch2/catch.hpp>

namespace
{
template < class T >
std::string
printed(T const &x)
{
    std::ostringstream ss;
    ss << x;
    return ss.str();
}

}   // namespace

TEST_CASE("property_map")
{
    using namespace boost::connector;

    auto m  = property_map();
    auto m2 = fix(mutable_property_map().set("base", "BTC").set("term", "USD"));
    auto m3 = fix(mutate(m2).set("venue", "ftx"));

    REQUIRE(m2.query("base"));
    REQUIRE(m2.query("base")->query<std::string>());
    CHECK(*m2.query("base")->query<std::string>() == "BTC");

    REQUIRE(m2.query("term"));
    REQUIRE(m2.query("term")->query<std::string>());
    CHECK(*m2.query("term")->query<std::string>() == "USD");

    CHECK(m2.query("venue") == nullptr);

    REQUIRE(m3.query("term"));
    REQUIRE(m3.query("term")->query<std::string>());
    CHECK(*m3.query("term")->query<std::string>() == "USD");

    REQUIRE(m3.query("venue") != nullptr);
    REQUIRE(m3.query("venue")->query<std::string>());
    CHECK(*m3.query("venue")->query<std::string>() == "ftx");
}