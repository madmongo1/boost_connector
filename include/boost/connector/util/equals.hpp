#ifndef BOOST_CONNECTOR__UTIL__EQUALS__HPP
#define BOOST_CONNECTOR__UTIL__EQUALS__HPP

#include <boost/describe.hpp>

namespace boost::connector
{
/// Compute equality of any type which has been annotated with BOOST_DESCRIBE_XXX.
/// @note https://www.boost.org/doc/libs/1_77_0/libs/describe/doc/html/describe.html
///
template < class T,
           class Bd = boost::describe::describe_bases< T, boost::describe::mod_any_access >,
           class Md = boost::describe::describe_members< T, boost::describe::mod_any_access > >
bool
operator==(T const &t1, T const &t2)
{
    bool r = true;

    boost::mp11::mp_for_each< Bd >(
        [&](auto D)
        {
            using B = typename decltype(D)::type;
            r       = r && (B const &)t1 == (B const &)t2;
        });

    boost::mp11::mp_for_each< Md >([&](auto D) { r = r && t1.*D.pointer == t2.*D.pointer; });

    return r;
}

}   // namespace boost::connector

#endif
