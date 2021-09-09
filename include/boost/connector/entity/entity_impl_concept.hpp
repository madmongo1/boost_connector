#ifndef BOOST_CONNECTOR__ENTITY__ENTITY_IMPL_CONCEPT__HPP
#define BOOST_CONNECTOR__ENTITY__ENTITY_IMPL_CONCEPT__HPP

#include <memory>

namespace boost::connector
{
struct entity_impl_concept
{
    virtual void
    start() = 0;

    virtual void
    stop() = 0;

    virtual ~entity_impl_concept() = default;
};

using weak_entity_impl_ptr = std::weak_ptr< entity_impl_concept >;
using entity_impl_ptr      = std::shared_ptr< entity_impl_concept >;

}   // namespace boost::connector

#endif
