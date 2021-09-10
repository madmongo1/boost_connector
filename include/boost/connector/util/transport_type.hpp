#ifndef BOOST_CONNECTOR__UTIL__TRANSPORT_TYPE__HPP
#define BOOST_CONNECTOR__UTIL__TRANSPORT_TYPE__HPP

#include <boost/describe.hpp>

#include <string>

namespace boost::connector
{
enum class transport_type
{
    tcp,
    tls
};
BOOST_DESCRIBE_ENUM(transport_type, tcp, tls)

}   // namespace boost::connector

#endif
