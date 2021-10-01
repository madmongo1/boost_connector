//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/boost_connector
//

#ifndef BOOST_CONNECTOR_PROPERTY_VALUE_HPP
#define BOOST_CONNECTOR_PROPERTY_VALUE_HPP

#include <boost/functional/hash.hpp>
#include <boost/utility/string_view.hpp>

#include <ostream>
#include <typeinfo>

namespace boost::connector
{
namespace detail
{
struct value_info
{
    std::type_info const *type;
    void const           *address;
};

template < class T >
struct compare_equal
{
    bool
    operator()(const void *lp, value_info vi) const
    {
        return typeid(T) == *vi.type &&
               std::equal_to< T >()(*static_cast< T const * >(lp),
                                    *static_cast< T const * >(vi.address));
    }
};

}   // namespace detail

struct property_value_jump_table
{
    const void *(*query)(const void *pv, std::type_info const &ti);
    detail::value_info (*info)(void const *pv);
    void (*destroy)(void *o);
    void (*move_construct)(void *dest, void *source);
    bool (*equal)(void const *l, detail::value_info vi);
    size_t (*hash_value)(void const *l);
    void (*to_ostream)(std::ostream &os, void const *arg);
};

inline const property_value_jump_table property_value_void_jump_table = {
    // clang-format off
    .query = [](const void* pv, std::type_info const& ti) -> void const * {
        return ti == typeid(void) ? pv : nullptr;
    },
    .info = [](void const* pv)
    {
        return detail::value_info { .type = &typeid(void), .address = pv };
    },
    .destroy = [](void *) {
    },
    .move_construct = [](void *, void *) {
    },
    .equal = [](void const       *,
                detail::value_info vi)
    {
        return typeid(void) == *vi.type;
    },
    .hash_value = [](void const *) -> size_t {
        return 0;
    },
    .to_ostream = [](std::ostream &os, void const *) {
        os << "empty";
    }
    // clang-format on
};

namespace detail
{
template < class T >
void
destroy_impl(T *p)
{
    p->~T();
}
}   // namespace detail

template < class T >
const property_value_jump_table property_value_short_jump_table = {
    // clang-format off
    .query = [](const void* pv, std::type_info const& ti) -> void const * {
        return ti == typeid(T) ? pv : nullptr;
    },
    .info = [](void const* pv)
    {
        return detail::value_info { .type = &typeid(T), .address = pv };
    },
    .destroy = [](void *vp)
    {
        detail::destroy_impl(static_cast< T* >(vp));
    },
    .move_construct = [](void *vpdest, void *vpsource)
    {
        T* pdest= static_cast< T* >(vpdest);
        T* psource= static_cast< T* >(vpsource);
        new (pdest) T (std::move(*psource));
    },
    .equal = [](void const       * lp,
                detail::value_info vi)
    {
        return detail::compare_equal<T>()(lp, vi);
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

template < class T >
const property_value_jump_table property_value_long_jump_table = {
    // clang-format off
    .query = [](const void* pv, std::type_info const& ti) -> void const * {
        auto up = static_cast<std::unique_ptr<T> const*>(pv);
        return ti == typeid(T) ? up->get() : nullptr;
    },
    .info = [](void const* pv)
    {
        return detail::value_info
        {
            .type = &typeid(T),
            .address = static_cast<std::unique_ptr<T> const*>(pv)->get()
        };
    },
    .destroy = [](void *vp)
    {
        detail::destroy_impl(static_cast<std::unique_ptr<T> *>(vp));
    },
    .move_construct = [](void *vpdest, void *vpsource)
    {
        std::unique_ptr<T> * pdest =
            static_cast< std::unique_ptr<T> * >(vpdest);
        std::unique_ptr<T> * psource =
            static_cast< std::unique_ptr<T> * >(vpsource);
        new (pdest) std::unique_ptr<T> (std::move(*psource));
    },
    .equal = [](void const       * vpl,
                detail::value_info vi)
    {
        std::unique_ptr<T> const& pl =
            *static_cast< std::unique_ptr<T> const * >(vpl);
        return detail::compare_equal<T>()(pl.get(), vi);
    },
    .hash_value = [](void const *vp)
    {
        std::unique_ptr<T> const* p  =
            static_cast< std::unique_ptr<T> const * >(vp);
        return boost::hash< T >()(**p);
    },
    .to_ostream = [](std::ostream &os, void const *vp)
    {
        std::unique_ptr<T> const *p  =
            static_cast< std::unique_ptr<T> const * >(vp);
        os << **p;
    }
    // clang-format on
};

/// @brief A polymorphic type which contains any object supporting the
/// concept of a property_type
/// @details A property type is move-aware, equality-comparable, hashable,
/// and ostreamable
/// @note This class supports Small Buffer Optimisation and will not
/// allocate memory when storing any object up to and including the size of
/// a string.
struct property_value
{
    /// @brief Construct from any type supporting the concept property_type.
    /// @note This overload id not a candidate for overload resolution if T
    /// is derived from property_value.
    /// @tparam T is any type supporting the property_type concept, other
    /// than property_value.
    /// @param x is a value of type T
    /// @post this->query<T>() != nullptr
    ///
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

    /// @brief Construct a property_value containing the void type.
    /// @post this->query<void>() != nullptr
    /// @post this->query<[anything other than void]>() == nullptr
    ///
    property_value()
    : jt_(&property_value_void_jump_table)
    , sbo_ {}
    {
    }

    /// @brief Move-construct a property_value from another.
    /// @post other.query<void>() != nullptr
    /// @post this object contains the state that was originally in other.
    /// @param other is the source of the move-construction.
    ///
    property_value(property_value &&other) noexcept;

    /// @brief Construct a property_value from a string literal.
    /// @post this->query<std::string>() != nullptr
    /// @post This object contains a std::string costructed from the string
    /// literal
    ///
    template < std::size_t N >
    property_value(const char (&s)[N])
    : property_value(std::string(s))
    {
    }

    /// @brief Construct a property_value from a string literal.
    /// @post this->query<std::string>() != nullptr
    /// @post This object contains a std::string constructed from the string
    /// literal
    ///
    property_value(const char *s);

    /// @brief Construct a property_value from a string_view.
    /// @post this->query<std::string>() != nullptr
    /// @post This object contains a std::string constructed from the
    /// string_view
    ///
    property_value(string_view s);

    /// @brief Move-assignement operator
    /// @post other.query<void>() != nullptr
    /// @post the original contents of this property_value is destroyed and
    /// original internal state of other is moved into *this.
    property_value &
    operator=(property_value &&other) noexcept;

    /// @brief Destructor
    ~property_value();

    friend std::ostream &
    operator<<(std::ostream &os, property_value const &arg);

    friend std::size_t
    hash_value(property_value const &arg);

    bool
    operator==(property_value const &r) const;

    template < class T >
    T const *
    query() const
    {
        const auto vi = jt_->info(&sbo_);
        if (typeid(T) == *vi.type)
            return static_cast< T const * >(vi.address);
        else
            return nullptr;
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
            new (&sbo_) std::unique_ptr< type >(new type(std::forward< T >(x)));
            jt_ = &property_value_long_jump_table< T >;
        }
    }

    using storage_type = std::aligned_storage_t< sizeof(std::string) >;
    storage_type                     sbo_;
    property_value_jump_table const *jt_;
};
}   // namespace boost::connector
#endif   // BOOST_CONNECTOR_PROPERTY_VALUE_HPP
