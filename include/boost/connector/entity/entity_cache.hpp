#ifndef BOOST_CONNECTOR__ENTITY__ENTITY_CACHE__HPP
#define BOOST_CONNECTOR__ENTITY__ENTITY_CACHE__HPP

#include <boost/connector/dependency/dependency_map.hpp>
#include <boost/connector/entity/entity_impl_concept.hpp>
#include <boost/connector/entity/entity_key.hpp>
#include <boost/connector/entity/lifetime_ptr.hpp>
#include <boost/connector/property_map.hpp>

#include <any>
#include <memory>
#include <typeindex>
#include <unordered_map>

namespace boost::connector
{
struct entity_cache_impl
{
    lifetime_ptr< entity_impl_concept >
    locate(std::type_info const &iface_type, entity_key const &args);

  private:
    using weak_lifetime_by_key =
        std::unordered_map< entity_key,
                            weak_lifetime_ptr< entity_impl_concept > >;

    using weak_lifetimes_by_interface =
        std::unordered_map< std::type_index, weak_lifetime_by_key >;

    weak_lifetimes_by_interface lifetime_cache_;
};

struct entity_context
{
    void
    set_dependencies(dependency_map m)
    {
        dependencies_ = std::move(m);
    }

    property_map const &
    params() const;

    void
    params(property_map &&pm);

  private:
    std::shared_ptr< entity_cache_impl > cache_;
    property_map                         properties_;
    dependency_map                       dependencies_;
};
}   // namespace boost::connector

#endif
