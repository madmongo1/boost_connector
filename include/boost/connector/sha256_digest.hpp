//
// Created by hodge on 24/11/2021.
//

#ifndef BOOST_CONNECTOR_SHA256_DIGEST_HPP
#define BOOST_CONNECTOR_SHA256_DIGEST_HPP

#include <boost/functional/hash.hpp>

#include <functional>
#include <iosfwd>
#include <span>

namespace boost::connector
{
struct sha256_digest
{
    static constexpr std::size_t digest_length = 32;

    using mutable_span = std::span< unsigned char, digest_length >;

    using const_span = std::span< unsigned char const, digest_length >;

    const_span
    data() const
    {
        return const_span(data_, data_ + digest_length);
    }

    mutable_span
    data()
    {
        return mutable_span(data_, data_ + digest_length);
    }

    bool
    operator==(sha256_digest const &r) const
    {
        return std::memcmp(data_, r.data_, sizeof(data_)) == 0;
    }

  private:
    friend std::size_t
    hash_value(sha256_digest const &self);

    friend std::ostream &
    operator<<(std::ostream &os, sha256_digest const &digest);

  private:
    unsigned char data_[digest_length];
};

}   // namespace boost::connector

namespace std
{
template <>
struct hash< boost::connector::sha256_digest >
: boost::hash< boost::connector::sha256_digest >
{
};

}   // namespace std

#endif   // BOOST_CONNECTOR_SHA256_DIGEST_HPP
