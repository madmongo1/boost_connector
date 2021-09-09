#ifndef BOOST_CONNECTOR__ENTITY__ENTITY_KEY__HPP
#define BOOST_CONNECTOR__ENTITY__ENTITY_KEY__HPP

#include <boost/functional/hash.hpp>

#include <memory>
#include <typeindex>

namespace boost::connector
{
struct entity_concept
{
    struct details
    {
        /// The type of the stored object
        std::type_index type;

        /// The address of the stored object
        void const *address;
    };

    virtual details
    get_details() const = 0;

    virtual bool
    test_equal(details other) const = 0;

    virtual std::size_t
    compute_hash() const = 0;
};

template < class T >
struct entity_model : entity_concept
{
    entity_model(T x)
    : model_(std::move(x))
    {
    }

    virtual details
    get_details() const override final
    {
        return details { .type = typeid(model_), .address = static_cast< void const * >(std::addressof(model_)) };
    }

    virtual bool
    test_equal(details other) const override final
    {
        auto const me = get_details();
        return me.type == other.type &&
               *static_cast< T const * >(me.address) == *static_cast< T const * >(other.address);
    }

    virtual std::size_t
    compute_hash() const override final
    {
        std::size_t seed = 0;
        boost::hash_combine(seed, typeid(T));
        boost::hash_combine(seed, model_);
        return seed;
    }

  private:
    T model_;
};

/// An object that contains any object conforming to the concept of EntityKeyObject.
/// In summary, objects of this type must be equality comparable, hashable and printable.
struct entity_key
{
    entity_key() = default;

    template < class EntityKeyType, std::enable_if_t< !std::is_base_of_v< entity_key, EntityKeyType > > * = nullptr >
    entity_key(EntityKeyType &&value);

    friend bool
    operator==(entity_key const &l, entity_key const &r);

    friend std::size_t
    hash_value(entity_key const &arg);

    /// Query whether the enclose object is exactly of type T.
    ///
    /// @return If the cv-unqualified type exactly matches T, then the address of the stored object is returned,
    /// otherwise nullptr. If entity_key does not contain an object, return nullptr
    template < class T >
    T const *
    query() const;

  private:
    template < class T >
    auto
    construct_model(T &&x)
    {
        using model_type = entity_model< std::decay_t< T > >;
        return std::make_shared< model_type >(std::forward< T >(x));
    }

  private:
    std::shared_ptr< entity_concept > impl_;
};

}   // namespace boost::connector

namespace std
{
template <>
struct hash< boost::connector::entity_key >
{
    std::size_t
    operator()(boost::connector::entity_key const &arg) const
    {
        return hash_value(arg);
    }
};
}   // namespace std

namespace boost::connector
{
template < class EntityKeyType, std::enable_if_t< !std::is_base_of_v< entity_key, EntityKeyType > > * >
entity_key::entity_key(EntityKeyType &&value)
: impl_(construct_model(std::forward< EntityKeyType >(value)))
{
}

template < class T >
T const *
entity_key::query() const
{
    T const *result = nullptr;

    if (impl_)
    {
        auto d = impl_->get_details();
        if (typeid(T) == d.type)
            result = static_cast< T const * >(impl_->get_details().address);
    }

    return result;
}

}   // namespace boost::connector

#endif
