#ifndef BOOST_CONNECTOR_TYPES_PRICE_HPP
#define BOOST_CONNECTOR_TYPES_PRICE_HPP

#include <string>

namespace boost::connector
{
/// @brief A type holding a standardised code for a deliverable asset, e.g. BTC
/// = bitcoin
using asset_code = std::string;

/// @brief The type used for a price of a tradable instrument
using price_value = double;

inline price_value
get_price_value(price_value x)
{
    return x;
}

struct qualified_price
{
    asset_code  code;
    price_value price;
};

}   // namespace boost::connector

#endif   // BOOST_CONNECTOR_TYPES_PRICE_HPP