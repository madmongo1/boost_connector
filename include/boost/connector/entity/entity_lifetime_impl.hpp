#ifndef BOOST_CONNECTOR__ENTITY__ENTITY_LIFETIME_IMPL__HPP
#define BOOST_CONNECTOR__ENTITY__ENTITY_LIFETIME_IMPL__HPP

#include <boost/connector/entity/entity_impl_concept.hpp>

namespace boost::connector
{
struct entity_lifetime_impl
{
    entity_lifetime_impl(std::shared_ptr< entity_impl_concept > ptr)
    : ptr_(std::move(ptr))
    {
        ptr_->start();
    }

    entity_lifetime_impl(entity_lifetime_impl const &) = delete;

    entity_lifetime_impl &
    operator=(entity_lifetime_impl const &) = delete;

    ~entity_lifetime_impl()
    {
        if (ptr_)
            ptr_->stop();
    }

    entity_impl_concept *
    entity_address() const
    {
        return ptr_.get();
    }

    std::shared_ptr< entity_impl_concept > ptr_;
};

}   // namespace boost::connector
#endif
