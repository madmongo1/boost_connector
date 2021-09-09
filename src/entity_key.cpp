#include <boost/connector/entity/entity_key.hpp>

namespace boost::connector
{
bool
operator==(entity_key const &l, entity_key const &r)
{
    const auto pl = l.impl_.get();
    const auto pr = r.impl_.get();

    if (pl == pr)
        return true;

    if (!pl || !pr)
        return false;

    return pl->test_equal(pr->get_details());
}

std::size_t
hash_value(entity_key const &arg)
{
    std::size_t seed = 0;

    if (arg.impl_)
        seed = arg.impl_->compute_hash();

    return seed;
}

}   // namespace boost::connector