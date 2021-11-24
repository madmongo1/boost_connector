//
// Created by hodge on 24/11/2021.
//

#ifndef BOOST_CONNECTOR_JSONEXT_HPP
#define BOOST_CONNECTOR_JSONEXT_HPP

#include <boost/json/string.hpp>
#include <boost/json/value.hpp>
#include <openssl/sha.h>

#include <vector>

namespace boost::connector::jsonext
{
void
update_sha256(SHA256_CTX *ctx, json::value const &val);

std::vector< json::string >
dissect_path(json::string const &input);

json::string
to_path(std::vector< json::string >::const_iterator first,
        std::vector< json::string >::const_iterator last);

json::string
to_path(std::vector< json::string > const &dissected);

json::value const &
find_path(json::value const                          &v,
          std::vector< json::string >::const_iterator original,
          std::vector< json::string >::const_iterator last);

json::value const &
find_path(json::value const &v, std::vector< json::string > const &path);

json::value const &
find_path(json::value const &v, json::string const &path);

json::value const &
subvalue(json::value const &v, json::string const &name);

}   // namespace boost::connector::jsonext
#endif   // BOOST_CONNECTOR_JSONEXT_HPP
