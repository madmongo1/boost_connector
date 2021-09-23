//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#ifndef BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_UTIL_DESCRIBE_OPERATORS_HPP
#define BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_UTIL_DESCRIBE_OPERATORS_HPP

#include <boost/connector/util/equals.hpp>
#include <boost/connector/util/hash_value.hpp>
#include <boost/describe.hpp>
#include <boost/functional/hash.hpp>
#include <boost/utility/string_view.hpp>

#include <cstddef>
#include <ostream>

namespace boost::connector
{
/// implement ostream << for any type annotated with describe
/// @note
/// https://www.boost.org/doc/libs/1_77_0/libs/describe/doc/html/describe.html
///
template <
    class T,
    class Bd =
        boost::describe::describe_bases< T, boost::describe::mod_any_access >,
    class Md = boost::describe::
        describe_members< T, boost::describe::mod_any_access > >
void
stream_out(std::ostream &os, T const &t, std::string_view &sep)
{
    boost::mp11::mp_for_each< Bd >(
        [&](auto D)
        {
            using B = typename decltype(D)::type;
            os << ((B const &)t);
        });

    boost::mp11::mp_for_each< Md >(
        [&](auto D)
        {
            os << sep << (t.*D.pointer);
            sep = ", ";
        });
}

template <
    class T,
    class Bd =
        boost::describe::describe_bases< T, boost::describe::mod_any_access >,
    class Md = boost::describe::
        describe_members< T, boost::describe::mod_any_access > >
std::ostream &
operator<<(std::ostream &os, T const &t)
{
    std::string_view sep = "";
    stream_out(os, t, sep);
    return os;
}

template < class F, class E, class DE = describe::describe_enumerators< E > >
void
visit_descriptor(F f, E e)
{
    mp11::mp_for_each< DE >(
        [&](auto D)
        {
            if (e == D.value)
                f(D);
        });
}

template < class E, class DE = describe::describe_enumerators< E > >
std::ostream &
operator<<(std::ostream &os, E e)
{
    visit_descriptor([&os](auto d) { os << d.name; }, e);
    return os;
}

template < class E, class DE = describe::describe_enumerators< E > >
string_view
to_string(E e)
{
    string_view r = "(unnamed)";
    visit_descriptor([&r](auto d) { r = d.name; }, e);
    return r;
}

}   // namespace boost::connector

#define BOOST_CONNECTOR_IMPLEMENT_DESCRIBE_EQUALS(T)                           \
    inline bool operator==(T const &t1, T const &t2)                           \
    {                                                                          \
        using Bd = ::boost::describe::                                         \
            describe_bases< T, ::boost::describe::mod_any_access >;            \
        using Md = ::boost::describe::                                         \
            describe_members< T, ::boost::describe::mod_any_access >;          \
        bool r = true;                                                         \
                                                                               \
        ::boost::mp11::mp_for_each< Bd >(                                      \
            [&](auto D)                                                        \
            {                                                                  \
                using B = typename decltype(D)::type;                          \
                r       = r && (B const &)t1 == (B const &)t2;                 \
            });                                                                \
                                                                               \
        ::boost::mp11::mp_for_each< Md >(                                      \
            [&](auto D) { r = r && t1.*D.pointer == t2.*D.pointer; });         \
                                                                               \
        return r;                                                              \
    }

#define BOOST_CONNECTOR_IMPLEMENT_DESCRIBE_HASH_VALUE(T)                       \
    inline std::size_t hash_value(T const &t)                                  \
    {                                                                          \
        using Bd = ::boost::describe::                                         \
            describe_bases< T, ::boost::describe::mod_any_access >;            \
        using Md = ::boost::describe::                                         \
            describe_members< T, ::boost::describe::mod_any_access >;          \
        std::size_t r = 0;                                                     \
                                                                               \
        ::boost::mp11::mp_for_each< Bd >(                                      \
            [&](auto D)                                                        \
            {                                                                  \
                using B = typename decltype(D)::type;                          \
                ::boost::hash_combine(r, (B const &)t);                        \
            });                                                                \
                                                                               \
        ::boost::mp11::mp_for_each< Md >(                                      \
            [&](auto D) { ::boost::hash_combine(r, t.*D.pointer); });          \
                                                                               \
        return r;                                                              \
    }

#define BOOST_CONNECTOR_IMPLEMENT_DESCRIBE_EQUALITY(T)                         \
    BOOST_CONNECTOR_IMPLEMENT_DESCRIBE_EQUALS(T)                               \
    BOOST_CONNECTOR_IMPLEMENT_DESCRIBE_HASH_VALUE(T)                           \
    inline bool operator!=(T const &t1, T const &t2) { return !(t1 == t2); }

#endif   // BOOST_CONNECTOR_INCLUDE_BOOST_CONNECTOR_UTIL_DESCRIBE_OPERATORS_HPP
