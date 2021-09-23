#ifndef BOOST_CONNECTOR__INTERFACE__ORDER_BOOK_CONCEPT__HPP
#define BOOST_CONNECTOR__INTERFACE__ORDER_BOOK_CONCEPT__HPP

#include <boost/connector/entity/entity_impl_concept.hpp>
#include <boost/connector/interface/orderbook_snapshot.hpp>

namespace boost::connector::interface
{
struct order_book_concept : entity_impl_concept
{
    using snapshot_type = orderbook_snapshot;

    virtual ~order_book_concept() = default;
};
}   // namespace boost::connector::interface

#endif
