#ifndef BOOST_CONNECTOR__INTERFACE__ORDER_BOOK_CONCEPT__HPP
#define BOOST_CONNECTOR__INTERFACE__ORDER_BOOK_CONCEPT__HPP

#include <boost/connector/entity/entity_impl_concept.hpp>

namespace boost::connector::interface
{
struct order_book_concept : entity_impl_concept
{
    // entity_impl_concept
    virtual void
    start() override = 0;

    virtual void
    stop() override = 0;
};
}   // namespace boost::connector::interface

#endif
