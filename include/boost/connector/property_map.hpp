//
// Created by hodge on 29/09/2021.
//

#ifndef BOOST_CONNECTOR_PROPERTY_MAP_HPP
#define BOOST_CONNECTOR_PROPERTY_MAP_HPP

#include <boost/functional/hash.hpp>

#include <ostream>
#include <unordered_map>

namespace boost::connector
{
/*
template < class V = boost::type_erasure::_self >
struct has_hash_value
{
    static std::size_t
    apply(V const &v)
    {
        const auto op = boost::hash< V >();
        return op(v);
    }
};

template < class T = boost::type_erasure::_self, class U = T && >
struct has_move_assignable
{
    static T &
    apply(T &t, U u)
    {
        return t = std::move(u);
    }
};
*/
}   // namespace boost::connector
/*
namespace boost::type_erasure
{
template < class C, class Base >
struct concept_interface< ::boost::connector::has_hash_value< C >, Base, C >
: Base
{
    friend std::size_t
    hash_value(const Base &x)
    {
        return call(::boost::connector::has_hash_value< C >(), x);
    }
};

template < class T, class U, class Base >
struct concept_interface< ::boost::connector::has_move_assignable< T, U >,
                          Base,
                          T > : Base
{
    Base &
    operator=(U &&y)
    {
        return call(
            ::boost::connector::has_move_assignable< T, U >(), *this, y);
    }
};

}   // namespace boost::type_erasure
 */
namespace boost::connector
{
// clang-format off
/*
using property_value =
boost::type_erasure::any<
    boost::mpl::vector<
//        has_move_assignable<>,
        boost::type_erasure::destructible<>,
        boost::type_erasure::equality_comparable<>,
        boost::type_erasure::constructible< boost::type_erasure::_self(
            boost::type_erasure::_self &&) >,
        boost::type_erasure::ostreamable<>,
        has_hash_value<>,
        boost::type_erasure::relaxed
    >
>;
*/
// clang-format on

struct property_value_jump_table
{
    std::type_info const &(*info)();
    void (*destroy)(void *o);
    void (*move_construct)(void *dest, void *source);
    bool (*equal)(void const *l, void const *r);
    std::size_t (*hash_value)(void const *l);
    void (*to_ostream)(std::ostream &os, void const *arg);
};

inline const property_value_jump_table property_value_void_jump_table = {
    // clang-format off
    .info = []() -> std::type_info const& {
        return typeid(void);
    },
    .destroy = [](void *) {
    },
    .move_construct = [](void *, void *) {
    },
    .equal = [](void const *, void const *) {
        return true;
    },
    .hash_value = [](void const *) -> std::size_t {
        return 0;
    },
    .to_ostream = [](std::ostream &os, void const *) {
        os << "empty";
    }
    // clang-format on
};

template < class T >
const property_value_jump_table property_value_short_jump_table = {
    // clang-format off
    .info = []() -> std::type_info const& { return typeid(T); },
    .destroy = [](void *vp)
    {
        T* p  = static_cast< T* >(vp);
        p->~T();
    },
    .move_construct = [](void *vpdest, void *vpsource)
    {
        T* pdest= static_cast< T* >(vpdest);
        T* psource= static_cast< T* >(vpsource);
        new (pdest) T (std::move(*psource));
    },
    .equal = [](void const *vpl, void const *vpr)
    {
        T const* pl = static_cast< T const * >(vpl);
        T const* pr = static_cast< T const * >(vpr);
        return *pl == *pr;
    },
    .hash_value = [](void const *vp)
    {
        T const* p  = static_cast< T const *>(vp);
        return boost::hash< T >()(*p);
    },
    .to_ostream = [](std::ostream &os, void const *vp)
    {
        T const *p  = static_cast< T const * >(vp);
        os << *p;
    }
    // clang-format on
};

struct property_value
{
    template < class T,
               std::enable_if_t<
                   !std::is_base_of_v< property_value, std::decay_t< T > > > * =
                   nullptr >
    property_value(T &&x)
    : sbo_ {}
    , jt_(&property_value_void_jump_table)
    {
        construct(std::forward< T >(x));
    }

    property_value()
    : jt_(&property_value_void_jump_table)
    , sbo_ {}
    {
    }

    property_value(property_value &&other)
    : jt_(&property_value_void_jump_table)
    , sbo_ {}
    {
        other.jt_->move_construct(&sbo_, &other.sbo_);
        jt_ = std::exchange(other.jt_, &property_value_void_jump_table);
    }

    template < std::size_t N >
    property_value(const char (&s)[N])
    : property_value(std::string(s))
    {
    }

    property_value &
    operator=(property_value &&other)
    {
        // make move-constructed copy
        auto tmp = property_value(std::move(other));

        // destroy our implementation
        jt_->destroy(&sbo_);
        jt_ = &property_value_void_jump_table;

        // move-construct our implementation from copy
        tmp.jt_->move_construct(&sbo_, &tmp);
        jt_ = std::exchange(tmp.jt_, &property_value_void_jump_table);

        return *this;
    }

    ~property_value() { jt_->destroy(&sbo_); }

    friend std::ostream &
    operator<<(std::ostream &os, property_value const &arg)
    {
        if (arg.jt_)
            arg.jt_->to_ostream(os, &arg.sbo_);
        else
            os << "empty";
        return os;
    }

    friend std::size_t
    hash_value(property_value const &arg)
    {
        if (arg.jt_)
            return arg.jt_->hash_value(&arg.sbo_);
        else
            return 0;
    }

    bool
    operator==(property_value const &r) const
    {
        if (jt_->info() != r.jt_->info())
            return false;
        return jt_->equal(&sbo_, &r.sbo_);
    }

    template < class T >
    void
    construct(T &&x)
    {
        using type = std::decay_t< T >;
        if (sizeof(type) <= sizeof(sbo_))
        {
            new (&sbo_) type(std::forward< T >(x));
            jt_ = &property_value_short_jump_table< type >;
        }
        else
        {
            jt_ = nullptr;
        }
    }

    using storage_type = std::aligned_storage_t< sizeof(std::string) >;
    storage_type                     sbo_;
    property_value_jump_table const *jt_;
};

struct property_map_layer
{
    using parent_type = std::shared_ptr< property_map_layer const >;

    property_map_layer(parent_type parent = nullptr)
    : parent_(std::move(parent))
    , map_()
    {
    }

    template < class T >
    void
    set(std::string_view name, T &&value)
    {
        static_assert(std::is_convertible_v< T &&, property_value >);
        static thread_local std::string key;
        key.assign(name.begin(), name.end());

        auto i = map_.find(key);
        if (i == map_.end())
            map_.emplace_hint(i, key, property_value(std::forward< T >(value)));
        else
            i->second = property_value(std::forward< T >(value));
    }

    parent_type parent_;
    std::
        unordered_map< std::string, property_value, boost::hash< std::string > >
            map_;
};

struct property_map
{
    property_map(std::shared_ptr< property_map_layer > impl = nullptr)
    : impl_(std::move(impl))
    {
    }

    std::shared_ptr< property_map_layer > const &
    get_implementation() const
    {
        return impl_;
    }

    std::shared_ptr< property_map_layer > impl_;
};

struct mutable_property_map
{
    mutable_property_map(property_map_layer::parent_type parent_layer)
    : impl_(std::make_shared< property_map_layer >(std::move(parent_layer)))
    {
    }

    template < class T >
    mutable_property_map &&
    set(std::string_view name, T &&value)
    {
        impl_->set(name, std::forward< T >(value));
        return std::move(*this);
    }

    template < class T >
    mutable_property_map &&
    set(std::string_view name, std::string_view sv)
    {
        impl_->set(name, std::string(sv.begin(), sv.end()));
        return std::move(*this);
    }

    template < class T >
    mutable_property_map &&
    set(std::string_view name, const char *sv)
    {
        impl_->set(name, std::string(sv));
        return std::move(*this);
    }

    std::shared_ptr< property_map_layer > &&
    get_implementation() &&
    {
        return std::move(impl_);
    }

    std::shared_ptr< property_map_layer > impl_;
};

inline mutable_property_map
mutate(property_map const &source_map)
{
    return mutable_property_map(source_map.get_implementation());
}

property_map
fix(mutable_property_map &&m)
{
    return property_map(std::move(m).get_implementation());
}

}   // namespace boost::connector
#endif   // BOOST_CONNECTOR_PROPERTY_MAP_HPP
