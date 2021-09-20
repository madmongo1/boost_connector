#include <catch2/catch.hpp>

#include <memory>
#include <typeinfo>

namespace boost::connector
{
/// @brief A deleter object

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

template < class T >
auto
make_lifetime_ptr(std::shared_ptr< T > const &impl)
{
    // Build a deleter containing a shared_ptr to the original impl
    auto deleter = lifetime_deleter< T > { .private_impl = impl };

    auto proxy = std::shared_ptr< bool >(new bool(false), std::move(deleter));

    impl->start();
    *proxy = true;

    return std::shared_ptr< T >(proxy, impl.get());
}
}   // namespace boost::connector

namespace
{
struct t1_state
{
    int started   = 0;
    int stopped   = 0;
    int destroyed = 0;
};

struct t1 : std::enable_shared_from_this< t1 >
{
    t1(t1_state &s)
    : state(s)
    {
    }

    ~t1() { state.destroyed += 1; }

    void
    start()
    {
        state.started += 1;
    }
    void
    stop()
    {
        state.stopped += 1;
    };

    std::shared_ptr< t1 >
    clone()
    {
        return shared_from_this();
    }

    t1_state &state;
};
}   // namespace

TEST_CASE("lifetime pointer")
{
    using boost::connector::make_lifetime_ptr;

    t1_state impl_state;
    {
        auto impl = std::make_shared< t1 >(impl_state);
        CHECK(impl_state.started == 0);
        CHECK(impl_state.stopped == 0);
        CHECK(impl_state.destroyed == 0);

        auto lifetime = make_lifetime_ptr(impl);

        CHECK(impl_state.started == 1);
        CHECK(impl_state.stopped == 0);
        CHECK(impl_state.destroyed == 0);

        {
            auto l2 = lifetime;
            CHECK(impl_state.started == 1);
            CHECK(impl_state.stopped == 0);
            CHECK(impl_state.destroyed == 0);
        }
        CHECK(impl_state.started == 1);
        CHECK(impl_state.stopped == 0);
        CHECK(impl_state.destroyed == 0);

        {
            auto i2 = impl;
            CHECK(impl_state.started == 1);
            CHECK(impl_state.stopped == 0);
            CHECK(impl_state.destroyed == 0);
        }
        CHECK(impl_state.started == 1);
        CHECK(impl_state.stopped == 0);
        CHECK(impl_state.destroyed == 0);

        auto i2 = impl->clone();

        auto different_control_block =
            []< class T >(std::shared_ptr< T > const &x,
                          std::shared_ptr< T > const &y)
        {
            if (x.owner_before(y))
                return true;
            if (y.owner_before(x))
                return true;
            return false;
        };

        CHECK(different_control_block(lifetime, impl));
        CHECK(different_control_block(lifetime, i2));
        CHECK(!different_control_block(impl, i2));

        lifetime.reset();

        CHECK(impl_state.started == 1);
        CHECK(impl_state.stopped == 1);
        CHECK(impl_state.destroyed == 0);
    }
    CHECK(impl_state.destroyed == 1);
}