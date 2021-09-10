#ifndef BOOST_CONNECTOR__UTIL__HASH_VALUE__HPP
#define BOOST_CONNECTOR__UTIL__HASH_VALUE__HPP

#include <boost/describe.hpp>
#include <boost/functional/hash.hpp>

namespace boost::connector
{
/// Compute the hash value of any type which has been annotated with BOOST_DESCRIBE_XXX.
/// @note https://www.boost.org/doc/libs/1_77_0/libs/describe/doc/html/describe.html
///
template < class T,
           class Bd = boost::describe::describe_bases< T, boost::describe::mod_any_access >,
           class Md = boost::describe::describe_members< T, boost::describe::mod_any_access > >
std::size_t
hash_value(T const &t)
{
    std::size_t r = 0;

    boost::mp11::mp_for_each< Bd >(
        [&](auto D)
        {
            using B = typename decltype(D)::type;
            boost::hash_combine(r, (B const &)t);
        });

    boost::mp11::mp_for_each< Md >([&](auto D) { boost::hash_combine(r, t.*D.pointer); });

    return r;
}

}   // namespace boost::connector

#endif
