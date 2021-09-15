#ifndef BOOST_CONNECTOR__INTERFACE__PRICE_LADDER__HPP
#define BOOST_CONNECTOR__INTERFACE__PRICE_LADDER__HPP

#include <boost/connector/entity/entity_impl_concept.hpp>

namespace boost::connector
{
struct price_ladder_concept
: entity_impl_concept
, std::enable_shared_from_this< price_ladder_concept >
{
    virtual void
    start() override;

    virtual void
    stop() override;
};

struct price_ladder
{
};
}   // namespace boost::connector

#endif
