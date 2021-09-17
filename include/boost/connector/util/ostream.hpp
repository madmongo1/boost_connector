#ifndef BOOST_CONNECTOR__UTIL__OSTREAM__HPP
#define BOOST_CONNECTOR__UTIL__OSTREAM__HPP

#include <boost/describe.hpp>

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

}   // namespace boost::connector

#endif
