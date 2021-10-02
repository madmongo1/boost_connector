//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/boost_connector
//

#include <boost/connector/property_value.hpp>
#include <catch2/catch.hpp>

#include <ostream>
#include <utility>

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

TEST_CASE("property_value - void")
{
    using namespace boost::connector;

    auto pv1 = property_value();
    auto pv2 = property_value();
    CHECK(printed(pv1) == "empty");
    CHECK(pv1 == pv2);
    CHECK(hash_value(pv1) == hash_value(pv2));
    CHECK(pv1.query< int >() == nullptr);
    CHECK(pv1.query< void >() != nullptr);
}

TEST_CASE("property_value - small")
{
    using namespace boost::connector;

    auto pv1 = property_value(std::string("foo"));
    auto pv2 = property_value(std::string("foo"));
    auto pv3 = property_value(std::string("bar"));
    CHECK(printed(pv1) == "foo");
    CHECK(pv1 == pv2);
    CHECK(pv1 != pv3);
    CHECK(hash_value(pv1) == hash_value(pv2));
    CHECK(pv1.query< int >() == nullptr);
    REQUIRE(pv1.query< std::string >() != nullptr);
    CHECK(*pv1.query< std::string >() == "foo");

    pv2 = std::move(pv3);
    REQUIRE(pv2.query< std::string >() != nullptr);
    CHECK(*pv2.query< std::string >() == "bar");
    CHECK(pv3.query< std::string >() == nullptr);
    CHECK(pv3.query< void >() != nullptr);
}

namespace
{
struct big_type
{
    big_type(const char *p)
    : data_ { 0 }
    {
        std::copy(
            p, p + (std::min< std::size_t >)(std::strlen(p), 1024), data_);
    }
    char data_[1024];

    bool
    operator==(big_type const &r) const
    {
        return std::memcmp(data_, r.data_, sizeof(data_)) == 0;
    }
};

std::ostream &
operator<<(std::ostream &os, big_type const &r)
{
    if (r.data_[1023])
        os.write(r.data_, 1024);
    else
        os << r.data_;
    return os;
}

std::size_t
hash_value(big_type const &arg)
{
    return boost::hash_range(std::begin(arg.data_), std::end(arg.data_));
}
}   // namespace

TEST_CASE("property_value - big")
{
    using namespace boost::connector;

    auto pv1 = property_value(big_type("foo"));
    auto pv2 = property_value(big_type("foo"));
    auto pv3 = property_value(big_type("bar"));
    CHECK(printed(pv1) == "foo");
    CHECK(pv1 == pv2);
    CHECK(pv1 != pv3);
    CHECK(hash_value(pv1) == hash_value(pv2));
    CHECK(pv1.query< int >() == nullptr);
    REQUIRE(pv1.query< big_type >() != nullptr);
    CHECK(*pv1.query< big_type >() == big_type("foo"));

    pv2 = std::move(pv3);
    CHECK(pv3.query< void >() != nullptr);
    CHECK(pv3.query< big_type >() == nullptr);
    REQUIRE(pv2.query< big_type >() != nullptr);
    CHECK(*pv2.query< big_type >() == big_type("bar"));
}

#include <boost/core/demangle.hpp>

namespace
{
struct ptype_base
{
    std::type_info const &ti;

    friend std::string
    to_string(ptype_base const &pt)
    {
        return boost::core::demangle(pt.ti.name());
    }

    friend bool
    operator==(ptype_base const &l, ptype_base const &r) noexcept
    {
        return l.ti == r.ti;
    }

    friend std::ostream &
    operator<<(std::ostream &l, ptype_base const &r) noexcept
    {
        return l << to_string(r);
    }
};

template < class T >
struct ptype : ptype_base
{
    ptype()
    : ptype_base { .ti = typeid(T) }
    {
    }
};
}   // namespace
TEST_CASE("property_value and strings")
{
    using namespace boost;

    std::string s   = "various strings";
    auto        pv1 = connector::property_value(s);
    auto        pv2 = connector::property_value(s.c_str());
    auto        pv3 = connector::property_value(s.data());
    auto        pv4 = connector::property_value(string_view(s));

    REQUIRE(pv1.query<std::string>() != nullptr);
    CHECK(*pv1.query<std::string>() == s);

    REQUIRE(pv2.query<std::string>() != nullptr);
    CHECK(*pv2.query<std::string>() == s);

    REQUIRE(pv3.query<std::string>() != nullptr);
    CHECK(*pv3.query<std::string>() == s);

    REQUIRE(pv4.query<std::string>() != nullptr);
    CHECK(*pv4.query<std::string>() == s);
}