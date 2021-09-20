#ifndef BOOST_CONNECTOR__ENTITY__LIFETIME_PTR__HPP
#define BOOST_CONNECTOR__ENTITY__LIFETIME_PTR__HPP

#include <memory>

namespace boost::connector
{
template < class T >
using lifetime_ptr = std::shared_ptr< T >;

template < class T >
using weak_lifetime_ptr = std::weak_ptr< T >;

namespace detail
{
template < class T >
struct lifetime_deleter
{
    void
    operator()(bool *pb) const noexcept
    {
        if (*pb)
            private_impl->stop();
        delete pb;
    }

    std::shared_ptr< T > private_impl {};
};

}   // namespace detail

template < class T >
lifetime_ptr< T >
make_lifetime_ptr(std::shared_ptr< T > const &impl)
{
    // Build a deleter containing a shared_ptr to the original impl
    auto deleter = detail::lifetime_deleter< T > { .private_impl = impl };

    auto proxy = std::shared_ptr< bool >(new bool(false), std::move(deleter));

    impl->start();
    *proxy = true;

    return lifetime_ptr< T >(proxy, impl.get());
}

template < class T, class... Args >
lifetime_ptr< T >
make_lifetime_ptr(Args &&...args)
{
    auto impl = std::make_shared< T >(std::forward< Args >(args)...);
    return make_lifetime_ptr(impl);
}

}   // namespace boost::connector
#endif
