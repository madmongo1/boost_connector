#ifndef BOOST_CONNECTOR_ENTITY_MUTABLE_KEY_HPP
#define BOOST_CONNECTOR_ENTITY_MUTABLE_KEY_HPP

#include <boost/connector/entity/key_base.hpp>

namespace boost::connector
{
struct immutable_key;

struct mutable_key : key_base
{
    mutable_key(std::shared_ptr< json::value const > original,
                path_set                             index = {});

    json::value const &
    use(json::string const &path);

    void
    merge(immutable_key const &other);

    immutable_key
    fix() const;

  private:
    static json::string
    normalise(json::string const &in);
};

}

#include <boost/connector/entity/impl/mutable_key.hpp>

#endif // BOOST_CONNECTOR_ENTITY_MUTABLE_KEY_HPP