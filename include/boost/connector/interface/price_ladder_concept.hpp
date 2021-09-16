#ifndef BOOST_CONNECTOR__INTERFACE__PRICE_LADDER__HPP
#define BOOST_CONNECTOR__INTERFACE__PRICE_LADDER__HPP

#include <boost/connector/entity/entity_impl_concept.hpp>

namespace boost::connector::interface
{
struct price_ladder_concept
: entity_impl_concept
{
    // entity_impl_concept
    virtual void
    start() override = 0;

    virtual void
    stop() override = 0;
};
}   // namespace boost::connector::interface

#endif
