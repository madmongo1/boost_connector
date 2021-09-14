#include <boost/connector/entity/entity_key.hpp>
#include <boost/functional/hash.hpp>
#include <catch2/catch.hpp>

TEST_CASE("entity_key", "[entity_key]")
{
    auto const expected      = std::string("foo");
    auto const expected_hash = boost::hash< std::string >()(expected);
    auto       k1            = boost::connector::entity_key(std::string("foo"));
    auto       k2            = boost::connector::entity_key(std::string("foo"));
    auto       k3            = boost::connector::entity_key(int(1));

    auto k1h = boost::hash< boost::connector::entity_key >()(k1);
    auto k2h = boost::hash< boost::connector::entity_key >()(k2);
    auto k3h = boost::hash< boost::connector::entity_key >()(k3);

    CHECK(k1h == k2h);
    CHECK(k1 == k2);
    CHECK(k1 != k3);
    CHECK(k1 == k1);
}