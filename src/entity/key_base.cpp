#include <boost/connector/entity/key_base.hpp>
#include <boost/connector/jsonext.hpp>
#include <boost/json/serialize.hpp>

namespace boost::connector
{
key_base::key_base(std::shared_ptr< const json::value > original,
                   key_base::path_set                   index)
: value_(std::move(original))
, index_(std::move(index))
{
}

std::ostream &
operator<<(std::ostream &os, const key_base &key)
{
    std::string_view sep = "";
    for (auto &path : key.index())
    {
        os << sep << path << "=" << jsonext::find_path(key.value(), path);
        sep = ", ";
    }
    return os;
}

}