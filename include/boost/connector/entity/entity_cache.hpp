#ifndef BOOST_CONNECTOR__ENTITY__ENTITY_CACHE__HPP
#define BOOST_CONNECTOR__ENTITY__ENTITY_CACHE__HPP

#include <boost/connector/entity/entity_key.hpp>
#include <boost/connector/entity/entity_lifetime_impl.hpp>

#include <typeindex>

namespace boost::connector
{
struct lifetime_map_key
{
    friend std::size_t
    hash_value(lifetime_map_key const &arg);

    friend bool
    operator==(lifetime_map_key const &l, lifetime_map_key const &r)
    {
        return l.interface_type_ == r.interface_type_ && l.key_ == r.key_;
    }

    std::type_index interface_type_;
    entity_key      key_;
};

struct entity_cache
{
    std::shared_ptr< entity_lifetime_impl >
    locate(std::type_info const &iface_type, entity_key const &args)
    {
        return nullptr;
    }
};
}   // namespace boost::connector

#endif
