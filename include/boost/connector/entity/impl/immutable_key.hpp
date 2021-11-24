//
// Created by hodge on 24/11/2021.
//

#ifndef BOOST_CONNECTOR_ENTITY_IMPL_IMMUTABLE_KEY_HPP
#define BOOST_CONNECTOR_ENTITY_IMPL_IMMUTABLE_KEY_HPP

namespace boost::connector
{
sha256_digest const &
immutable_key::digest() const
{
    return digest_;
}

bool
immutable_key::operator==(const immutable_key &r) const
{
    return digest_ == r.digest() && equal_contents(r);
}

}
#endif   // BOOST_CONNECTOR_IMMUTABLE_KEY_HPP
