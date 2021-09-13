#ifndef BOOST_CONNETOR__VENDOR__FTX__FTX_CONFIG__HPP
#define BOOST_CONNETOR__VENDOR__FTX__FTX_CONFIG__HPP

#include <boost/connector/util/equals.hpp>
#include <boost/connector/util/hash_value.hpp>
#include <boost/describe.hpp>

#include <string>
#include <tuple>

namespace boost::connector
{
struct ftx_credentials
{
    std::string subaccount;
    std::string api_key;
    std::string api_secret;

    bool
    has_auth() const
    {
        return !api_key.empty() && !api_secret.empty();
    }

    bool
    has_subaccount() const
    {
        return !subaccount.empty();
    }
};

BOOST_DESCRIBE_STRUCT(ftx_credentials, (), (subaccount, api_key, api_secret))

struct ftx_websocket_key
{
    std::string     url;
    ftx_credentials auth;
};

BOOST_DESCRIBE_STRUCT(ftx_websocket_key, (), (url, auth))

}   // namespace boost::connector

#endif
