#include <boost/connector/condition.hpp>
#include <catch2/catch.hpp>

TEST_CASE("boost::connector::condition")
{
    using namespace boost::connector;

    std::ostringstream ss;

    auto c = status_code::good;
    auto s = to_string(c);
    ss << c;
    CHECK(s == "good");
    CHECK(ss.str() == s);

    c = status_code::error;
    s = to_string(c);
    ss.str("");
    ss << c;
    CHECK(s == "error");
    CHECK(ss.str() == s);

    c = status_code::not_ready;
    s = to_string(c);
    ss.str("");
    ss << c;
    CHECK(s == "not_ready");
    CHECK(ss.str() == s);

    CHECK(status_code::good < status_code::not_ready);
    CHECK(status_code::not_ready < status_code::error);
}