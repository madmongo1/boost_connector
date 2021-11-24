#ifndef BOOST_CONNECTOR_ENTITY_IMPL_KEY_BASE_HPP
#define BOOST_CONNECTOR_ENTITY_IMPL_KEY_BASE_HPP

namespace boost::connector
{
json::value const &
key_base::value() const
{
    return *value_;
}

std::shared_ptr< json::value const > const &
key_base::value_ptr() const
{
    return value_;
}

key_base::const_iterator
key_base::begin() const
{
    return index_.begin();
}

key_base::const_iterator
key_base::end() const
{
    return index_.end();
}

const key_base::path_set &
key_base::index() const
{
    return index_;
}

key_base::path_set &
key_base::index()
{
    return index_;
}

}

#endif   // BOOST_CONNECTOR_ENTITY_IMPL_KEY_BASE_HPP
