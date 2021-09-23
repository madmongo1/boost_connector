#include <boost/connector/snapshot/snapshot.hpp>
#include <catch2/catch.hpp>

TEST_CASE("boost::connector::snapshot")
{
    using namespace boost::connector;

    auto s1 = snapshot {};

    CHECK(s1.status == status_code::error);

    auto s2 = snapshot {};

    CHECK(equivalent(s1, s2));
    s1.source = "s1";
    s2.source = "s2";

    s1.status = status_code::good;

    CHECK(!equivalent(s1, s2));
}
