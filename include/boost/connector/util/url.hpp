#ifndef BOOST_CONNECTOR__UTIL__URL__HPP
#define BOOST_CONNECTOR__UTIL__URL__HPP

#include <boost/connector/util/transport_type.hpp>
#include <boost/describe.hpp>

#include <string>

namespace boost::connector
{
struct url_parts
{
    std::string    hostname;
    std::string    service;
    std::string    path_etc;
    transport_type transport;
};

/// decode a url into component parts that we can use
url_parts
decode_url(std::string const &url);
}   // namespace boost::connector

#endif
