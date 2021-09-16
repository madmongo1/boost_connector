#ifndef BOOST_CONNECTOR__ENTITY__LIFETIME_PTR__HPP
#define BOOST_CONNECTOR__ENTITY__LIFETIME_PTR__HPP

#include <boost/connector/entity/entity_lifetime_impl.hpp>

namespace boost::connector
{
template < class EntityImpl >
struct lifetime_ptr;

template < class EntityImpl >
struct weak_lifetime_ptr;

template < class EntityImpl >
struct lifetime_ptr
{
    explicit lifetime_ptr(std::shared_ptr< entity_lifetime_impl > impl = {})
    : ptr_(nullptr)
    , lifetime_impl_(std::move(impl))
    {
        if (lifetime_impl_)
            ptr_ = static_cast< EntityImpl * >(lifetime_impl_->entity_address());
    }

    template < class U, std::enable_if_t< std::is_convertible_v< U *, EntityImpl * > > * = nullptr >
    lifetime_ptr(lifetime_ptr< U > const &src)
    : ptr_(static_cast< U * >(src.get()))
    , lifetime_impl_(src.get_lifetime_impl())
    {
    }

    EntityImpl *
    get() const
    {
        return ptr_;
    }

    operator bool() const
    {
        return get() != nullptr;
    }


    EntityImpl *
    operator->() const
    {
        return get();
    }

    EntityImpl &
    operator*() const
    {
        return *get();
    }

    void
    reset()
    {
        lifetime_impl_.reset();
        ptr_ = nullptr;
    }

    template < class U >
    friend struct weak_lifetime_ptr;

    std::shared_ptr< entity_lifetime_impl > const &
    get_lifetime_impl() const
    {
        return lifetime_impl_;
    }

  private:
    EntityImpl                             *ptr_;
    std::shared_ptr< entity_lifetime_impl > lifetime_impl_;
};

template < class EntityImpl >
struct weak_lifetime_ptr
{
    weak_lifetime_ptr()
    : lifetime_impl_()
    {
    }

    weak_lifetime_ptr(lifetime_ptr< EntityImpl > const &src)
    : lifetime_impl_(src.lifetime_impl_)
    {
    }

    lifetime_ptr< EntityImpl >
    lock()
    {
        return lifetime_ptr< EntityImpl >(lifetime_impl_.lock());
    }

  private:
    std::weak_ptr< entity_lifetime_impl > lifetime_impl_;
};

template < class EntityImpl, class... Args >
lifetime_ptr< EntityImpl >
make_lifetime_ptr(Args &&...args)
{
    auto impl = std::make_shared< EntityImpl >(std::forward< Args >(args)...);
    auto lifetime_base =
        std::make_shared< entity_lifetime_impl >(std::static_pointer_cast< entity_impl_concept >(impl));
    return lifetime_ptr< EntityImpl >(std::move(lifetime_base));
}

}   // namespace boost::connector
#endif
