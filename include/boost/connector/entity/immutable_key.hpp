#ifndef BOOST_CONNECTOR_ENTITY_IMMUTABLE_KEY_HPP
#define BOOST_CONNECTOR_ENTITY_IMMUTABLE_KEY_HPP

#include <boost/connector/entity/key_base.hpp>
#include <boost/connector/sha256_digest.hpp>

namespace boost::connector
{
struct mutable_key;

struct immutable_key : key_base
{
    immutable_key(std::shared_ptr< json::value const > original,
                  path_set                             idx = {});

    mutable_key
    mutate() const;

    inline sha256_digest const &
    digest() const;

    inline bool
    operator==(immutable_key const &r) const;

  private:
    friend std::size_t
    hash_value(immutable_key const &self);

    bool
    equal_contents(immutable_key const &r) const;

  private:
    sha256_digest digest_;
};

}   // namespace boost::connector

namespace std
{
template <>
struct hash< ::boost::connector::immutable_key >
: ::boost::hash< ::boost::connector::immutable_key >
{
};

}   // namespace std

#include <boost/connector/entity/impl/immutable_key.hpp>

#endif   // BOOST_CONNECTOR_ENTITY_IMMUTABLE_KEY_HPP