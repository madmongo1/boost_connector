#include <boost/connector/entity/immutable_key.hpp>
#include <boost/connector/entity/mutable_key.hpp>
#include <boost/connector/jsonext.hpp>

namespace boost::connector
{
mutable_key::mutable_key(std::shared_ptr< json::value const > original,
                         path_set                             index)
: key_base(std::move(original), std::move(index))
{
}

json::value const &
mutable_key::use(const json::string &path)
try
{
    // diect the path to normalise it
    auto dissected = jsonext::dissect_path(path);

    // find on the dissected representation
    json::value const &ref = jsonext::find_path(value(), dissected);

    // convert the dissected path to canonical representation for indexing
    index().insert(jsonext::to_path(dissected));

    // return found value reference
    return ref;
}
catch (std::exception &)
{
    std::throw_with_nested(std::invalid_argument(
        "mutable_key::use: " + std::string(path.begin(), path.end())));
}

void
mutable_key::merge(const immutable_key &other)
{
    for (auto &&path : other)
    {
        auto [i, b] = index().insert(path);
        boost::ignore_unused(i, b);
    }
}

immutable_key
mutable_key::fix() const
{
    return immutable_key(value_ptr(), index());
}

json::string
mutable_key::normalise(const json::string &in)
{
    return jsonext::to_path(jsonext::dissect_path(in));
}

}   // namespace boost::connector